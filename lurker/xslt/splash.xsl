<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="splash" mode="title">
 <xsl:copy-of select="$splash"/>
</xsl:template>

<xsl:template match="list">
 <tr>
  <td class="center">
   <xsl:if test="number(messages) > 0">
    <a href="../mindex/{id}@{offset}.{$ext}">
     <xsl:value-of select="email/@name"/>
    </a>
   </xsl:if>
   <xsl:if test="number(messages) = 0">
    <xsl:value-of select="email/@name"/>
   </xsl:if>
  </td>
  <xsl:if test="description"><td><xsl:value-of select="description"/></td></xsl:if>
  <xsl:if test="not(description)"><td>--</td></xsl:if>
 </tr>
</xsl:template>

<xsl:template match="list" mode="select">
  <option value="{id}"><xsl:value-of select="email/@name"/>
  <xsl:if test="email/@address">
   <xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>
   &lt;<xsl:value-of select="email/@address"/>&gt;
  </xsl:if>
  </option>
</xsl:template>

<xsl:template match="splash" mode="body">
 <div id="listblock">
 <xsl:if test="count(list)&lt;8">
  <table class="listtable0">
   <tr class="header"><th width="35%"><xsl:value-of select="$lin-h"/></th>
                      <th width="65%"><xsl:value-of select="$des-h"/></th></tr>
   <xsl:apply-templates select="list"/>
  </table>
 </xsl:if>

 <xsl:if test="count(list)>7">
  <div id="listtable1">
   <table>
    <tr class="header"><th width="30%"><xsl:value-of select="$lin-h"/></th>
                       <th width="70%"><xsl:value-of select="$des-h"/></th></tr>
    <xsl:for-each select="list">
     <xsl:if test="position()&lt;=ceiling(last() div 2)">
      <xsl:if test="not(position()=ceiling(last() div 2))">
       <tr class="list">
        <td class="center">
          <a href="../mindex/{id}@{offset}.{$ext}">
          <xsl:value-of select="email/@name"/></a></td>
        <xsl:if test="description"><td><xsl:value-of select="description"/></td></xsl:if>
        <xsl:if test="not(description)"><td>--</td></xsl:if>
       </tr>
      </xsl:if>
      <xsl:if test="position()=ceiling(last() div 2)">
       <tr class="list">
        <td class="center">
         <a href="../mindex/{id}@{offset}.{$ext}">
         <xsl:value-of select="email/@name"/></a></td>
        <xsl:if test="description"><td><xsl:value-of select="description"/></td></xsl:if>
        <xsl:if test="not(description)"><td>--</td></xsl:if>
       </tr>
      </xsl:if>
     </xsl:if>
    </xsl:for-each>
   </table>
  </div>
  <div id="listtable2">
   <table>
    <tr class="header"><th width="30%"><xsl:value-of select="$lin-h"/></th>
                       <th width="70%"><xsl:value-of select="$des-h"/></th></tr>
    <xsl:for-each select="list">
     <xsl:if test="position()>ceiling(last() div 2)">
      <xsl:if test="not(position()=last())">
       <tr class="list"> 
        <td class="center">
         <a href="../mindex/{id}@{offset}.{$ext}">
         <xsl:value-of select="email/@name"/></a></td>
        <xsl:if test="description"><td><xsl:value-of select="description"/></td></xsl:if>
        <xsl:if test="not(description)"><td>--</td></xsl:if>
       </tr>
      </xsl:if>
      <xsl:if test="position()=last()">
       <tr>
        <td class="center">
         <a href="../mindex/{id}@{offset}.{$ext}">
         <xsl:value-of select="email/@name"/></a></td>
        <xsl:if test="description"><td><xsl:value-of select="description"/></td></xsl:if>
        <xsl:if test="not(description)"><td>--</td></xsl:if>
       </tr>
      </xsl:if>
     </xsl:if>
    </xsl:for-each>
   </table>
  </div>
 </xsl:if>
 </div>

 <br/><br/><hr/>
 <h2>Lurker <xsl:copy-of select="$search"/></h2>

 <form action ="../lurker-search.cgi">
  <input type="hidden" name="format" value="{$ext}"/>
  <input type="text"   name="query"  value="" size="50"/>
  <input type="submit" name="submit" value="{$search}!"/>
  <hr class="search"/>
  <table class="search">
   <tr><th><xsl:copy-of select="$author"/></th>
       <td class="center"><input type="text" name="author"  size="53" value=""/></td></tr>
   <tr><th><xsl:copy-of select="$subject"/></th>
       <td class="center"><input type="text" name="subject" size="53" value=""/></td></tr>
   <tr><th><xsl:copy-of select="$startdate"/></th>
    <td>
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
