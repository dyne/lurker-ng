<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

<xsl:template match="list" mode="select">
 <option value="{id}">
  <xsl:value-of select="email/@name"/>
  <xsl:if test="email/@address">
   <xsl:text>&#160;&lt;</xsl:text>
   <xsl:value-of select="email/@address"/>
   <xsl:text>&gt;</xsl:text>
  </xsl:if>
 </option>
</xsl:template>

<xsl:template match="list">
 <xsl:if test="number(messages) > 0">
  <a href="../mindex/{offset}@{id}.{$ext}">
   <xsl:value-of select="email/@name"/>
  </a>
 </xsl:if>
 <xsl:if test="number(messages) = 0">
  <xsl:value-of select="email/@name"/>
 </xsl:if>
</xsl:template>

<xsl:template name="format">
 <xsl:param name="col1"/>
 <xsl:param name="col2"/>
 <xsl:param name="col3"/>
 <xsl:param name="col4"/>
 <xsl:param name="exist1"/>
 <xsl:param name="exist2"/>
 <xsl:param name="exist3"/>
 <xsl:param name="exist4"/>
 <xsl:if test="count($col1) &gt; 0 or count($col2) &gt; 0 or count($col3) &gt; 0 or count($col4) &gt; 0">
  <xsl:text>
  </xsl:text>
  <tr>
   <xsl:if test="count($exist1) &gt; 0">
    <td nowrap="NOWRAP"><xsl:apply-templates select="$col1[position() = 1]"/></td>
   </xsl:if>
   <xsl:if test="count($exist2) &gt; 0">
    <td nowrap="NOWRAP"><xsl:apply-templates select="$col2[position() = 1]"/></td>
   </xsl:if>
   <xsl:if test="count($exist3) &gt; 0">
    <td nowrap="NOWRAP"><xsl:apply-templates select="$col3[position() = 1]"/></td>
   </xsl:if>
   <xsl:if test="count($exist4) &gt; 0">
    <td nowrap="NOWRAP"><xsl:apply-templates select="$col4[position() = 1]"/></td>
   </xsl:if>
  </tr>
  <xsl:call-template name="format">
   <xsl:with-param name="col1" select="$col1[position() &gt; 1]"/>
   <xsl:with-param name="col2" select="$col2[position() &gt; 1]"/>
   <xsl:with-param name="col3" select="$col3[position() &gt; 1]"/>
   <xsl:with-param name="col4" select="$col4[position() &gt; 1]"/>
   <xsl:with-param name="exist1" select="$exist1"/>
   <xsl:with-param name="exist2" select="$exist2"/>
   <xsl:with-param name="exist3" select="$exist3"/>
   <xsl:with-param name="exist4" select="$exist4"/>
  </xsl:call-template>
 </xsl:if>
</xsl:template>

<xsl:template match="splash" mode="title">
 <xsl:value-of select="$splash"/>
</xsl:template>

<xsl:template match="splash" mode="body">
 <div id="listBlock">
 <table id="listTable" width="100%">
  <caption>
   <xsl:value-of select="$lists"/>
   <xsl:text> (</xsl:text>
   <xsl:value-of select="sum(list/messages)"/>
   <xsl:text> </xsl:text>
   <xsl:value-of select="$mess"/>
   <xsl:text>)</xsl:text>
  </caption>
  
  <xsl:variable name="size1" select="round((count(list)+3) div 4)"/>
  <xsl:variable name="size2" select="round((count(list)+2) div 4)"/>
  <xsl:variable name="size3" select="round((count(list)+1) div 4)"/>
  <xsl:variable name="size4" select="round((count(list)+0) div 4)"/>
  
  <xsl:variable name="off1" select="0"/>
  <xsl:variable name="off2" select="$off1+$size1"/>
  <xsl:variable name="off3" select="$off2+$size2"/>
  <xsl:variable name="off4" select="$off3+$size3"/>
  <xsl:variable name="off5" select="$off4+$size4"/>

  <xsl:call-template name="format">
   <xsl:with-param name="col1" select="list[position() &gt;= $off1 and position() &lt; $off2]"/>
   <xsl:with-param name="col2" select="list[position() &gt;= $off2 and position() &lt; $off3]"/>
   <xsl:with-param name="col3" select="list[position() &gt;= $off3 and position() &lt; $off4]"/>
   <xsl:with-param name="col4" select="list[position() &gt;= $off4 and position() &lt; $off5]"/>
   <xsl:with-param name="exist1" select="list[position() &gt;= $off1 and position() &lt; $off2]"/>
   <xsl:with-param name="exist2" select="list[position() &gt;= $off2 and position() &lt; $off3]"/>
   <xsl:with-param name="exist3" select="list[position() &gt;= $off3 and position() &lt; $off4]"/>
   <xsl:with-param name="exist4" select="list[position() &gt;= $off4 and position() &lt; $off5]"/>
  </xsl:call-template>
 </table>
 </div>

 <br/><br/><hr/>
 <h2>Lurker <xsl:value-of select="$search"/></h2>

 <div id="searchBlock">
 <form action ="../lurker-search.cgi">
  <input type="hidden" name="format" value="{$ext}"/>
  <input type="text" name="query" class="longtext"/>
  <input type="submit" name="submit" value="{$search}!"/>
 <hr id="searchHr"/>
  <table id="searchTable">
   <tr><th><xsl:value-of select="$subject"/></th>
       <td><input type="text" name="subject" class="longtext"/></td></tr>
   <tr><th><xsl:value-of select="$author"/></th>
       <td><input type="text" name="author" class="longtext"/></td></tr>
   <tr>
    <th><xsl:value-of select="$appearinlist"/></th>
    <td>
     <select name="list" id="listdrop">
      <option value=""><xsl:value-of select="$all-li"/></option>
      <xsl:apply-templates mode="select" select="list"/>
     </select>
    </td>
   </tr>
   <tr>
    <td colspan="2">
     <input type="hidden" name="start-sec" value="0"/>
     <input type="hidden" name="start-min" value="0"/>
     <input type="hidden" name="start-hour" value="0"/>
     <select name="start-mday">
       <xsl:call-template name="option-range">
         <xsl:with-param name="start">1</xsl:with-param>
         <xsl:with-param name="last">31</xsl:with-param>
         <xsl:with-param name="select">1</xsl:with-param>
       </xsl:call-template>
     </select><xsl:value-of select="$date1"/>
     <select name="start-mon">
       <option value="1" selected="SELECTED"><xsl:value-of select="$jan"/></option>
       <option value="2"><xsl:value-of select="$feb"/></option>
       <option value="3"><xsl:value-of select="$mar"/></option>
       <option value="4"><xsl:value-of select="$apr"/></option>
       <option value="5"><xsl:value-of select="$may"/></option>
       <option value="6"><xsl:value-of select="$jun"/></option>
       <option value="7"><xsl:value-of select="$jul"/></option>
       <option value="8"><xsl:value-of select="$aug"/></option>
       <option value="9"><xsl:value-of select="$sep"/></option>
       <option value="10"><xsl:value-of select="$oct"/></option>
       <option value="11"><xsl:value-of select="$nov"/></option>
       <option value="12"><xsl:value-of select="$dec"/></option>
     </select><xsl:value-of select="$date2"/>
     <select name="start-year">
       <xsl:call-template name="option-range">
         <xsl:with-param name="start">1980</xsl:with-param>
         <xsl:with-param name="last">2003</xsl:with-param>
         <xsl:with-param name="select">1980</xsl:with-param>
       </xsl:call-template>
     </select>

     <xsl:text> </xsl:text><xsl:value-of select="$start-end"/><xsl:text> </xsl:text>

     <input type="hidden" name="end-sec" value="59"/>
     <input type="hidden" name="end-min" value="59"/>
     <input type="hidden" name="end-hour" value="23"/>
     <select name="end-mday">
       <xsl:call-template name="option-range">
         <xsl:with-param name="start">1</xsl:with-param>
         <xsl:with-param name="last">31</xsl:with-param>
         <xsl:with-param name="select">31</xsl:with-param>
       </xsl:call-template>
     </select><xsl:value-of select="$date1"/>
     <select name="end-mon">
       <option value="1"><xsl:value-of select="$jan"/></option>
       <option value="2"><xsl:value-of select="$feb"/></option>
       <option value="3"><xsl:value-of select="$mar"/></option>
       <option value="4"><xsl:value-of select="$apr"/></option>
       <option value="5"><xsl:value-of select="$may"/></option>
       <option value="6"><xsl:value-of select="$jun"/></option>
       <option value="7"><xsl:value-of select="$jul"/></option>
       <option value="8"><xsl:value-of select="$aug"/></option>
       <option value="9"><xsl:value-of select="$sep"/></option>
       <option value="10"><xsl:value-of select="$oct"/></option>
       <option value="11"><xsl:value-of select="$nov"/></option>
       <option value="12" selected="SELECTED"><xsl:value-of select="$dec"/></option>
     </select><xsl:value-of select="$date2"/>
     <select name="end-year">
       <xsl:call-template name="option-range">
         <xsl:with-param name="start">1980</xsl:with-param>
         <xsl:with-param name="last">2003</xsl:with-param>
         <xsl:with-param name="select">2003</xsl:with-param>
       </xsl:call-template>
     </select>
    </td>
   </tr>
  </table>
 </form>
 </div>

 <br/><hr/>

 <h2><xsl:value-of select="$info"/></h2>

 <dl><xsl:apply-templates select="list" mode="longdesc"/></dl>

</xsl:template>

<xsl:template match="list" mode="longdesc">
 <dt>
  <xsl:value-of select="email/@name"/>
  <xsl:if test="email/@address">
   <xsl:text>&#160;</xsl:text>
   <xsl:element name="a">
    <xsl:attribute name="href">
     <xsl:text>mailto:</xsl:text>
     <xsl:apply-templates select="email" mode="mailto"/>
    </xsl:attribute>

    <xsl:text>(</xsl:text>
    <xsl:value-of select="email/@address"/>
    <xsl:text>)</xsl:text>
   </xsl:element>
  </xsl:if>
 </dt>
 <dd>
  <xsl:if test="number(messages) &gt; 0">
   [ <a href="../mindex/{offset}@{id}.{$ext}">
      <xsl:value-of select="messages"/>
      <xsl:text> </xsl:text>
      <xsl:value-of select="$mess"/>
     </a>
   ]
   [ <a href="../lurker-search.cgi?query=lt%3A{id}&amp;format={$ext}">
      <xsl:value-of select="$threads"/>
     </a>
   ]
  </xsl:if>
  <xsl:if test="number(messages) = 0">[ 0 <xsl:value-of select="$mess"/> ]</xsl:if>

  <xsl:text> -- </xsl:text>

  <xsl:if test="description"><span class="desc"><xsl:value-of select="description"/></span></xsl:if>
  <xsl:if test="not(description)"><span class="desc"><xsl:value-of select="$nodesc"/></span></xsl:if>
 </dd>
</xsl:template>

</xsl:stylesheet>