#define LIBXSLT_PUBLIC

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/DOCBparser.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

int main(int argc, char** argv)
{
	xsltStylesheetPtr	cur = 0;
	xmlDocPtr		doc, res;
	const char*		params[1];
	
	params[0] = 0;
	
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;
	cur = xsltParseStylesheetFile((const xmlChar*)argv[1]);
	
	doc = xmlParseFile("-");
	res = xsltApplyStylesheet(cur, doc, params);
	xsltSaveResultToFile(stdout, res, cur);
	
	xsltFreeStylesheet(cur);
	xmlFreeDoc(res);
	xmlFreeDoc(doc);
	xsltCleanupGlobals();
	xmlCleanupParser();
	
	return 0;
}
