<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="list" mode="select">
  <option value="{id}"><xsl:value-of select="email/@name"/>
  <xsl:if test="email/@address">
   <xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>
   &lt;<xsl:value-of select="email/@address"/>&gt;
  </xsl:if>
  </option>
</xsl:template>

<xsl:template match="list">
 <xsl:if test="number(messages) > 0">
  <a href="../mindex/{id}@{offset}.{$ext}">
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
 <xsl:if test="count($col1) &gt; 0 or count($col2) &gt; 0 or count($col3) &gt; 0 or count($col4) &gt; 0">
<xsl:text>
</xsl:text>
  <tr>
   <td><xsl:apply-templates select="$col1[position() = 1]"/></td>
   <td><xsl:apply-templates select="$col2[position() = 1]"/></td>
   <td><xsl:apply-templates select="$col3[position() = 1]"/></td>
   <td><xsl:apply-templates select="$col4[position() = 1]"/></td>
  </tr>
  <xsl:call-template name="format">
   <xsl:with-param name="col1" select="$col1[position() &gt; 1]"/>
   <xsl:with-param name="col2" select="$col2[position() &gt; 1]"/>
   <xsl:with-param name="col3" select="$col3[position() &gt; 1]"/>
   <xsl:with-param name="col4" select="$col4[position() &gt; 1]"/>
  </xsl:call-template>
 </xsl:if>
</xsl:template>

<xsl:template match="splash" mode="title">
 <xsl:copy-of select="$splash"/>
</xsl:template>

<xsl:template match="splash" mode="body">
 <table class="listtable0" width="100%">
  <tr class="header">
   <th colspan="4" width="35%"><xsl:value-of select="$lin-h"/></th>
  </tr>
  
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
   <xsl:with-param name="col1" select="list[position() &gt; $off1 and position() &lt;= $off2]"/>
   <xsl:with-param name="col2" select="list[position() &gt; $off2 and position() &lt;= $off3]"/>
   <xsl:with-param name="col3" select="list[position() &gt; $off3 and position() &lt;= $off4]"/>
   <xsl:with-param name="col4" select="list[position() &gt; $off4 and position() &lt;= $off5]"/>
  </xsl:call-template>
 </table>

 <br/><br/><hr/>
 <h2>Lurker <xsl:copy-of select="$search"/></h2>

 <form action ="../lurker-search.cgi">
  <input type="hidden" name="format" value="{$ext}"/>
  <div class="center">
   <input type="text"   name="query"  value="" size="50"/>
   <input type="submit" name="submit" value="{$search}!"/>
  </div>
  <hr class="search"/>
  <table class="search">
   <tr><th><xsl:copy-of select="$author"/></th>
       <td class="center"><input type="text" name="author"  size="53" value=""/></td></tr>
   <tr><th><xsl:copy-of select="$subject"/></th>
       <td class="center"><input type="text" name="subject" size="53" value=""/></td></tr>
   <tr><th><xsl:copy-of select="$startdate"/></th>
    <td>
     <input type="hidden" name="start-sec" value="0"/>
     <input type="hidden" name="start-min" value="0"/>
     <input type="hidden" name="start-hour" value="0"/>
     <xsl:value-of select="$thedate"/>
     <xsl:text> </xsl:text>
     <select name="start-mday">
       <xsl:call-template name="option-range">
         <xsl:with-param name="start">1</xsl:with-param>
         <xsl:with-param name="last">31</xsl:with-param>
         <xsl:with-param name="select">1</xsl:with-param>
       </xsl:call-template>
     </select>
     <xsl:text> </xsl:text>
     <xsl:value-of select="$ofdate"/>
     <xsl:text> </xsl:text>
     <select name="start-mon">
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
       <option value="12"><xsl:value-of select="$dec"/></option>
     </select>
     <xsl:text> </xsl:text>
     <xsl:value-of select="$datedivide"/>
     <xsl:text> </xsl:text>
     <select name="start-year">
       <xsl:call-template name="option-range">
         <xsl:with-param name="start">1980</xsl:with-param>
         <xsl:with-param name="last">2003</xsl:with-param>
         <xsl:with-param name="select">1980</xsl:with-param>
       </xsl:call-template>
     </select>
    </td>
   </tr>
   <tr><th><xsl:copy-of select="$enddate"/></th>
    <td>
     <input type="hidden" name="end-sec" value="59"/>
     <input type="hidden" name="end-min" value="59"/>
     <input type="hidden" name="end-hour" value="23"/>
     <xsl:value-of select="$thedate"/>
     <xsl:text> </xsl:text>
     <select name="end-mday">
       <xsl:call-template name="option-range">
         <xsl:with-param name="start">1</xsl:with-param>
         <xsl:with-param name="last">31</xsl:with-param>
         <xsl:with-param name="select">31</xsl:with-param>
       </xsl:call-template>
     </select>
     <xsl:text> </xsl:text>
     <xsl:value-of select="$ofdate"/>
     <xsl:text> </xsl:text>
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
       <option value="12" selected="yes"><xsl:value-of select="$dec"/></option>
     </select>
     <xsl:text> </xsl:text>
     <xsl:value-of select="$datedivide"/>
     <xsl:text> </xsl:text>
     <select name="end-year">
       <xsl:call-template name="option-range">
         <xsl:with-param name="start">1980</xsl:with-param>
         <xsl:with-param name="last">2003</xsl:with-param>
         <xsl:with-param name="select">2003</xsl:with-param>
       </xsl:call-template>
     </select>
    </td>
   </tr>
   <tr>
    <th><xsl:copy-of select="$appearinlist"/></th>
    <td>
     <select name="list">
      <option value=""><xsl:value-of select="$all-li"/></option>
      <xsl:apply-templates mode="select" select="list"/>
     </select>
    </td>
   </tr>
  </table>
 </form>

 <br/><hr/>

 <h2><xsl:value-of select="$info"/></h2>

 <dl><xsl:apply-templates select="list" mode="longdesc"/></dl>

</xsl:template>

<xsl:template match="list" mode="longdesc">
 <dt>
  <b><xsl:value-of select="email/@name"/></b>
  <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;</xsl:text>
  <xsl:if test="email/@address">
   <a href="mailto:{email/@address}">(<xsl:value-of select="email/@address"/>)</a>
  </xsl:if>
 </dt>
 <dd><xsl:if test="number(messages) &gt; 0">
      [ <a href="../mindex/{id}@{offset}.{$ext}">
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
     <xsl:if test="number(messages) = 0">
      [ 0 <xsl:value-of select="$mess"/> ]
     </xsl:if>
     <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;--&amp;nbsp;&amp;nbsp;</xsl:text>
     <I><xsl:value-of select="description"/></I></dd>
</xsl:template>

</xsl:stylesheet>
