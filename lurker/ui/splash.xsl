<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="common.xsl"/>


<!-- Format option menus -->
<xsl:template match="list" mode="list-select">
 <option value="{id}">
  <xsl:value-of select="email/@name"/>
  <xsl:if test="email/@address">
   <xsl:text>&#160;&lt;</xsl:text>
   <xsl:value-of select="email/@address"/>
   <xsl:text>&gt;</xsl:text>
  </xsl:if>
 </option>
</xsl:template>
<xsl:template match="group" mode="group-select">
 <option value="{id}">
  <xsl:choose>
   <xsl:when test="heading"><xsl:value-of select="heading"/></xsl:when>
   <xsl:otherwise><xsl:value-of select="id"/></xsl:otherwise>
  </xsl:choose>
 </option>
</xsl:template>


<!-- Format mailing list groups -->
<xsl:template match="list" mode="list-sections">
 <li>
  <b><a href="../list/{id}.{$ext}"><xsl:value-of select="email/@name"/></a></b>  &#8212; <i><xsl:value-of select="description"/></i>
 </li>
 <xsl:text>&#10;</xsl:text>
</xsl:template>
<xsl:template match="group" mode="group-sections">
 <xsl:if test="position()!=1">
  <div class="footer"/>
 </xsl:if>
 <div class="body">
  <h1 class="group">
   <a name="{id}"/>
    <xsl:choose>
     <xsl:when test="heading"><xsl:value-of select="heading"/></xsl:when>
     <xsl:otherwise><xsl:value-of select="id"/></xsl:otherwise>
    </xsl:choose>
  </h1>
  <ul>
   <xsl:apply-templates select="list" mode="list-sections"/>
  </ul>
 </div>
</xsl:template>


<!-- Format a splash request -->
<xsl:template match="splash">
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="../fmt/default.css" type="text/css"/>
   <title><xsl:value-of select="server/archive"/></title>
   <script type="text/javascript" src="../fmt/common.js"/>
  </head>
  <body>
   <div class="header">
    <table class="external">
     <tr>
      <td align="left"><h1><xsl:value-of select="$front-page"/></h1></td>
      <td align="right">
       <h1><xsl:apply-templates mode="splash-link" select="server"/></h1>
      </td>
     </tr>
    </table>
    
    <table class="navigation">
     <tr><th align="left"><xsl:value-of select="$search-menu"/></th></tr>
     <tr><td nowrap="NOWRAP" align="center">
      <form action="{server/cgi-url}/keyword.cgi" accept-charset="UTF-8" onsubmit="form_timezone(this)">
       <input type="hidden" name="doc-url" value="{server/doc-url}"/>
       <input type="hidden" name="format" value="{$ext}"/>
       
       <table>
        <tr>
         <td colspan="2">
          <input type="text" name="query" class="longtext"/>
          <input type="submit" name="submit" value="{$search}!"/>
         </td>
        </tr>
        
        <tr>
         <td><b><xsl:value-of select="$subject"/></b></td>
         <td><input type="text" name="subject" class="longtext"/></td>
        </tr>
        
        <tr>
         <td><b><xsl:value-of select="$author"/></b></td>
         <td><input type="text" name="author" class="longtext"/></td>
        </tr>
        
        <tr>
         <td><b><xsl:value-of select="$list"/></b></td>
         <td>
          <select name="list">
           <option value=""><xsl:value-of select="$all-lists"/></option>
           <xsl:for-each select="group/list">
            <xsl:sort select="email/@name"/>
            <xsl:apply-templates mode="list-select" select="."/>
           </xsl:for-each>
          </select>
         </td>
        </tr>
        
        <xsl:if test="count(group) > 1">
         <tr>
          <td><b><xsl:value-of select="$group"/></b></td>
          <td>
           <select name="group">
            <option value=""><xsl:value-of select="$all-groups"/></option>
            <xsl:apply-templates mode="group-select" select="group"/>
           </select>
          </td>
         </tr>
        </xsl:if>
       
        <tr>
         <td><b><xsl:value-of select="$date"/></b></td>
         <td>
          <xsl:call-template name="date-fields">
           <xsl:with-param name="date" select="$jump-date"/>
          </xsl:call-template>
         </td>
        </tr>
       </table>
      </form>
     </td></tr>
    </table>
   </div>
   
   <xsl:apply-templates mode="group-sections" select="group"/>
   
   <div class="footer">
    <xsl:call-template name="lurker-signature"/>
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
