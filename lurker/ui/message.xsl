<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="common.xsl"/>

<xsl:template match="message">
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="message.css" type="text/css"/>
   <title><xsl:value-of select="summary/subject"/></title>
  </head>
  <body>
   <div class="header">
    <table class="external">
     <tr>
      <td width="100%">
       <h1>
        <xsl:value-of select="summary/subject"/>
       </h1>
      </td>
      <td>
       <h1>
        <xsl:apply-templates mode="splash-link" select="server"/>
       </h1>
      </td>
     </tr>
    </table>
    
    <table class="navigation">
     <tr>
      <th width="100%">
       <xsl:value-of select="$tree-context"/>
      </th>
      <td>
       <a href="../thread/{summary/id}.{$ext}">
        <xsl:value-of select="$full-tree"/>
       </a>
      </td>
     </tr>
     <tr>
      <td nowrap="NOWRAP">
       <xsl:for-each select="threading/snippet/tree">
        <xsl:if test="not(position()=1)"><br/></xsl:if>
        <xsl:apply-templates mode="tree"/>
       </xsl:for-each>
      </td>
      <td nowrap="NOWRAP">
       <xsl:choose>
        <xsl:when test="threading/prev">
         <a href="{threading/prev/id}.{$ext}">
          <img src="../imgs/prev.png"/>
          <xsl:apply-templates mode="email-name" select="threading/prev/summary/email"/>
         </a>
        </xsl:when>
        <xsl:otherwise>&#160;</xsl:otherwise>
       </xsl:choose>
       <br/>
       <xsl:choose>
        <xsl:when test="threading/next">
         <a href="{threading/next/id}.{$ext}">
          <xsl:apply-templates mode="email-name" select="threading/next/summary/email"/>
          <img src="../imgs/next.png"/>
         </a>
        </xsl:when>
        <xsl:otherwise>&#160;</xsl:otherwise>
       </xsl:choose>
      </td>
     </tr>
    </table>
   </div>
   
   <div class="message">
    <table class="header">
    </table>
   </div>
   
   
   <div class="footer">
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
