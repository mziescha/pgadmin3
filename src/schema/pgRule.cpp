//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// Copyright (C) 2002, The pgAdmin Development Team
// This software is released under the pgAdmin Public Licence
//
// pgRule.cpp - Rule class
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "misc.h"
#include "pgObject.h"
#include "pgRule.h"
#include "pgCollection.h"


pgRule::pgRule(pgSchema *newSchema, const wxString& newName)
: pgSchemaObject(newSchema, PG_RULE, newName)
{
}

pgRule::~pgRule()
{
}



wxString pgRule::GetSql(wxTreeCtrl *browser)
{
    if (sql.IsNull())
    {
    }
    return sql;
}



void pgRule::ShowTreeDetail(wxTreeCtrl *browser, frmMain *form, wxListCtrl *properties, wxListCtrl *statistics, ctlSQLBox *sqlPane)
{
    if (properties)
    {
        CreateListColumns(properties);
        int pos=0;

        InsertListItem(properties, pos++, wxT("Name"), GetName());
        InsertListItem(properties, pos++, wxT("OID"), GetOid());
        InsertListItem(properties, pos++, wxT("Event"), GetEvent());
        InsertListItem(properties, pos++, wxT("Condition"), GetCondition());
        InsertListItem(properties, pos++, wxT("Do Instead?"), GetDoInstead());
        InsertListItem(properties, pos++, wxT("Action"), GetAction());
        InsertListItem(properties, pos++, wxT("Definition"), GetDefinition());
        InsertListItem(properties, pos++, wxT("System Rule?"), GetSystemObject());
        InsertListItem(properties, pos++, wxT("Comment"), GetComment());
    }
}



pgObject *pgRule::Refresh(wxTreeCtrl *browser, const wxTreeItemId item)
{
    pgObject *rule=0;
    wxTreeItemId parentItem=browser->GetItemParent(item);
    if (parentItem)
    {
        pgObject *obj=(pgObject*)browser->GetItemData(parentItem);
        if (obj->GetType() == PG_RULES)
            rule = ReadObjects((pgCollection*)obj, 0, wxT("\n   AND rw.oid=") + GetOidStr());
    }
    return rule;
}



pgObject *pgRule::ReadObjects(pgCollection *collection, wxTreeCtrl *browser, const wxString &restriction)
{
    pgRule *rule=0;

    pgSet *rules= collection->GetDatabase()->ExecuteSet(wxT(
        "SELECT oid, rulename, pg_get_ruledef(oid) as definition, is_instead, ev_type, ev_action, ev_qual\n"
        "  FROM pg_rewrite rw\n"
        " WHERE ev_class = ") + NumToStr(collection->GetOid()) 
        + restriction + wxT("::oid\n"
        " ORDER BY rulename"));

    if (rules)
    {
        while (!rules->Eof())
        {
            rule = new pgRule(collection->GetSchema(), rules->GetVal(wxT("rulename")));

            rule->iSetOid(rules->GetOid(wxT("oid")));
            rule->iSetTableOid(collection->GetOid());
            rule->iSetDefinition(rules->GetVal(wxT("definition")));
            rule->iSetDoInstead(rules->GetBool(wxT("is_instead")));
            rule->iSetAction(rules->GetVal(wxT("ev_action")));
            rule->iSetComment(rules->GetVal(wxT("ev_qual")));
            char *evts[] = {0, "SELECT", "UPDATE", "INSERT", "DELETE"};
            int evno=StrToLong(rules->GetVal(wxT("ev_type")));
            if (evno > 0 && evno < 5)
                rule->iSetEvent(evts[evno]);
            else
                rule->iSetEvent(wxT("Unknown"));

            if (browser)
            {
                browser->AppendItem(collection->GetId(), rule->GetFullName(), PGICON_RULE, -1, rule);
				rules->MoveNext();
            }
            else
                break;
        }

		delete rules;
    }
    return rule;
}



void pgRule::ShowTreeCollection(pgCollection *collection, frmMain *form, wxTreeCtrl *browser, wxListCtrl *properties, wxListCtrl *statistics, ctlSQLBox *sqlPane)
{
    if (browser->GetChildrenCount(collection->GetId(), FALSE) == 0)
    {
        // Log
        wxLogInfo(wxT("Adding Rules to schema ") + collection->GetSchema()->GetIdentifier());

        // Get the Rules
        ReadObjects(collection, browser);
    }
}

