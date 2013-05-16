/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2009 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */


/*
*Opc_client_ae driver (OPC Alarms and Events 1.10)
*
*/

#include <qt.h>
#include "opc_client_ae.h"
#include "opc_client_ae_instance.h"
#include "general_defines.h"


/*
*Function:Opc_client_ae
*Inputs:none
*Outputs:none
*Returns:none
*/
Opc_client_ae::Opc_client_ae(QObject *parent,const QString &name) : Driver(parent,name),n_opc_items(0)
{
	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database
};
/*
*Function:~Opc_client_ae
*Inputs:none
*Outputs:none
*Returns:none
*/
Opc_client_ae::~Opc_client_ae()
{
};
/*
*Function:UnitConfigure
*Inputs:parent widget, unit name
*Outputs:none
*Returns:none
*/
void Opc_client_ae::UnitConfigure(QWidget *parent, const QString &name, const QString &receipe) // configure a unit
{
	Opc_client_aeConfiguration dlg(parent,name,receipe);
	dlg.exec();   
};
/*
*Function:UnitConfigure
*Inputs:parent widget, unit name
*Outputs:none
*Returns:none
*/
void Opc_client_ae::CommandDlg(QWidget *parent, const QString &name) // command dialog
{
	Opc_client_aeCommand dlg(parent,name);
	dlg.exec();   
};
/*
*Function:SetTypeList
*Inputs:combo , unit name
*Outputs:none
*Returns:none
*/
void Opc_client_ae::SetTypeList(QComboBox *pCombo, const QString &unitname) // set the type list for unit type
{
	pCombo->insertItem(TYPE_M_SP_NA_1);
	pCombo->insertItem(TYPE_M_ME_TC_1);
	pCombo->insertItem(TYPE_M_SP_TA_1);
	pCombo->insertItem(TYPE_M_ME_NB_1);
	pCombo->insertItem(TYPE_M_ME_TB_1);
};
/*
*Function:GetInputList
*Inputs:type
*Outputs:list of input indices
*Returns:none
*/
void Opc_client_ae::GetInputList(const QString &type, QStringList &list,const QString &, const QString &) // set the permitted input IDs
{
/*
	if(type == TYPE_M_ME_TC_1)
	{
		list << "01" << "02" << "03" << "04" << "05" << "06" << "07" << "08" 
		<< "09" << "10" << "11" << "12" << "13" << "14" << "15" << "16";         
	}

	if(type == TYPE_M_SP_NA_1)
	{
		list << "01" << "02" << "03" << "04" << "05" << "06" << "07" << "08" 
		<< "09" << "10" << "11" << "12" << "13" << "14" << "15" << "16";         
	}
*/
};
/*
*Function:GetSpecificConfig
*Inputs:parent , sample point name, sample point type
*Outputs:none
*Returns:none
*/
QWidget * Opc_client_ae::GetSpecificConfig(QWidget *parent, const QString &spname, const QString &sptype) //specific config for sample point of type
{
	Opc_client_aeInput * p;
	if(sptype == TYPE_M_ME_TC_1)
	{
		p = new Opc_client_aeInput(parent,spname);
		return p;
	}
	else if (sptype == TYPE_M_SP_NA_1)
	{
		p = new Opc_client_aeInput(parent,spname);
		return p;
	}

	return 0;
};
/*
*Function:GetTagList
*Inputs:type
*Outputs:permitted tag list for unit
*Returns:none
*/
void Opc_client_ae::GetTagList(const QString &type, QStringList &list,const QString &,const QString &) // returns the permitted tags for a given type for this unit
{
	list << VALUE_TAG;
};
/*
*Function:Command
*pass a command to a unit
*Inputs:unit name, command
*Outputs:none
*Returns:none
*/
void Opc_client_ae::Command(const QString & instance,BYTE cmd, LPVOID lpPa, DWORD pa_length, DWORD ipindex)
{
	IT_IT("Opc_client_ae::Command");

	IDict::iterator i = Instances.find(instance);

	if(!(i == Instances.end()))
	{
		(*i).second->Command(instance, cmd, lpPa, pa_length, ipindex); // pass on the command to the instance of the driver
	};
};
/*
*Function:CreateNewUnit
*Inputs:parent widget , unit name
*Outputs:none
*Returns:none
*/
void Opc_client_ae::CreateNewUnit(QWidget *parent, const QString &name, int n_inputs) // create a new unit - quick configure
{
	n_opc_items = n_inputs;
	opc_unit_name = name;
	
	QString n;
	n.sprintf("%02d",1);
	QString spname = name+"Point"+n;
	
	QString pc = 
	"select * from PROPS where SKEY='SAMPLEPROPS' and IKEY='" + spname +"';"; 
	//
	// get the properties SKEY = unit name IKEY = receipe name
	GetConfigureDb()->DoExec(this,pc, tcreateNewUnit);
};
//
Opc_client_ae * Opc_client_ae::pDriver = 0; // only one instance should be created
/*
*Function: Start
*Inputs:none
*Outputs:none
*Returns:none
*/
void Opc_client_ae::Start() // start everything under this driver's control
{
		QString cmd = "select * from UNITS where UNITTYPE='opc_client_ae_driver' and NAME in(" + DriverInstance::FormUnitList()+ ");";
		GetConfigureDb()->DoExec(this,cmd,tListUnits);
};
/*
*Function: Stop
*Inputs:none
*Outputs:none
*Returns:none
*/
void Opc_client_ae::Stop() // stop everything under this driver's control
{
	// ask each instance to stop 
	IDict::iterator i = Instances.begin();

	for(; !(i == Instances.end());i++)
	{
		(*i).second->Stop();
	};
	//
	// now delete them
	//     
	i = Instances.begin();

	for(; !(i == Instances.end());i++)
	{
		delete (*i).second;
	};
	//  
	Instances.clear();
};
/*
*Function:QueryResponse
*Inputs:client object, command, transaction id
*Outputs:none
*Returns:none
*/
void Opc_client_ae::QueryResponse (QObject *p, const QString &, int id, QObject*caller)
{
	if(this != p) return;
	switch(id)
	{  
		case tListUnits:
		{
			int n = GetConfigureDb()->GetNumberResults();
			if(n > 0)
			{
				for(int i = 0; i < n; i++,GetConfigureDb()->FetchNext())
				{
					QString unit_name = GetConfigureDb()->GetString("NAME");
					Opc_client_ae_Instance *p = new Opc_client_ae_Instance(this, unit_name, i);
					IDict::value_type pr(unit_name, p);
					Instances.insert(pr);
					p->Start(); // kick it off 
				};
			};
		};
		break;
		case tcreateNewUnit:
		{
			if(GetConfigureDb()->GetNumberResults() == 0)
			{
				for(int i = 1 ; i <= n_opc_items; i++)
				{
					QString n;
					n.sprintf("%02d",i);
					QString spname = opc_unit_name+"Point"+n;
					//
					QString cmd = 
					QString("insert into SAMPLE values('") + spname + 
					QString("',' Point Number ")  + n + "','" + opc_unit_name + 
					QString("','"TYPE_M_ME_TC_1"','V',1,1,'") + n + "',0,0,0);";
					// 
					GetConfigureDb()->DoExec(0,cmd,0); // post it off

					QStringList l;
					GetTagList(TYPE_M_ME_TC_1,l,"",""); 

					QString m;
					m.sprintf("%d",i);

					CreateSamplePoint(spname, l, m);

					cmd = QString("update TAGS set IOA=");
					cmd += m;
					cmd += " where NAME='" + spname + "';";

					GetConfigureDb()->DoExec(0,cmd ,0);

					cmd = QString("update TAGS set UNIT='");
					cmd += opc_unit_name;
					cmd += "' where NAME='" + spname + "';";

					GetConfigureDb()->DoExec(0,cmd ,0);
				};
			}
		}
		break;
		default:
		break;
	};   
};

// ********************************************************************************************************************************
/*
*Function:GetDriverEntry
*Inputs:parent object
*Outputs:none
*Returns:driver interface 
*/
extern "C"
{ 
	#ifdef WIN32
	OPC_CLIENT_AEDRV Driver *  _cdecl _GetDriverEntry(QObject *parent); 
	OPC_CLIENT_AEDRV void _cdecl _Unload();
	#endif
	Driver *  _cdecl  _GetDriverEntry(QObject *parent) 
	{
		if(!Opc_client_ae::pDriver)
		{
			Opc_client_ae::pDriver = new Opc_client_ae(parent,"Opc_client_ae");
		};
		return Opc_client_ae::pDriver;
	};
	/*
	*Function: _Unload
	*clean up before DLL unload. and QObjects must be deleted or we get a prang
	*Inputs:none
	*Outputs:none
	*Returns:none
	*/
	void _cdecl _Unload()
	{
		if(Opc_client_ae::pDriver) delete Opc_client_ae::pDriver;
		Opc_client_ae::pDriver = 0;
	};
};
