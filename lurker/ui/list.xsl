<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="common.xsl"/>


<!-- Format a list request -->
<xsl:template match="list">
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="default.css" type="text/css"/>
   <title><xsl:value-of select="list/email/@name"/></title>
  </head>
  <body>
   <div class="header">
    <table class="external">
     <tr>
      <td width="100%">
       <h1><xsl:value-of select="list/email/@name"/></h1>
      </td>
      <td nowrap="NOWRAP">
       <h1><xsl:apply-templates mode="splash-link" select="server"/></h1>
      </td>
     </tr>
    </table>
    
    <table class="navigation">
     <tr><th colspan="3"><xsl:value-of select="$jump-to-date"/></th></tr>
     <tr><td width="50%">&#160;</td><td nowrap="NOWRAP">
      <form action="{server/cgi-url}/jump.cgi">
       <input type="hidden" name="doc-url" value="{server/doc-url}"/>
       <input type="hidden" name="format" value="{$ext}"/>
       <input type="hidden" name="list" value="{list/id}"/>
       
       <xsl:call-template name="date-fields">
        <xsl:with-param name="date" select="$jump-date"/>
       </xsl:call-template>
       <input type="submit" value="{$jump}!"/>
      </form>
     </td><td width="50%">&#160;</td></tr>
    </table>
   </div>
   
   
   <div class="body">
    <b><xsl:value-of select="list/email/@name"/></b>
    <xsl:if test="list/description">
     &#8212;
     <xsl:value-of select="list/description"/>
    </xsl:if>
    <br/>
    <xsl:if test="list/email/@address">
     [ <a href="mailto:{list/email/@address}"><xsl:value-of select="$post-new"/></a> ]
    </xsl:if>
    <xsl:if test="list/link">
     [ <a href="{list/link}"><xsl:value-of select="$subscribe"/></a> ]
    </xsl:if>
    
   </div>
   <div class="footer"/>
   <div class="body">
    <table>
     <tr>
      <th><xsl:value-of select="$thread"/></th>
      <th><xsl:value-of select="$recent-poster"/></th>
      <th><xsl:value-of select="$post-count"/></th>
      <th><xsl:value-of select="$activity-chart"/></th>
     </tr>
    </table>
   </div>
   
   
   <div class="footer">
    <table class="navigation">
     <tr><th colspan="3"><xsl:value-of select="$search-list"/></th></tr>
     <tr><td width="50%">&#160;</td><td nowrap="NOWRAP">
      <form action="{server/cgi-url}/keyword.cgi" accept-charset="UTF-8">
       <input type="hidden" name="doc-url" value="{server/doc-url}"/>
       <input type="hidden" name="format"  value="{$ext}"/>
       <input type="text"   name="query"   value="ml:{list/id} " class="longtext"/>
       <input type="submit" name="submit"  value="{$search}!"/><br/>
       <xsl:value-of select="$use-special-word-list"/>
       <b>ml:<xsl:value-of select="list/id"/></b>
       <xsl:value-of select="$to-search-list"/>
      </form>
     </td><td width="50%">&#160;</td></tr>
    </table>
    
    <table class="external">
     <tr>
      <td class="mini">
       <b><a href="{$lurker-url}">Lurker</a></b>
       (version <xsl:value-of select="server/version"/>)
      </td>
      <td class="mini" align="right">
       <xsl:value-of select="$admin-by"/>
       <xsl:apply-templates mode="email-link" select="server/email"/>
      </td>
     </tr>
    </table>
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
