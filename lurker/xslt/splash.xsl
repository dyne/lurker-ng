<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:template match="splash" mode="title">
 <xsl:apply-templates select="server" mode="title"/> -
 <xsl:copy-of select="$splash"/>
</xsl:template>

<xsl:template match="list">
 <tr>
  <td class="center">
   <a href="../mindex/{id}@{string(floor((number(messages) - 1) div 20)*20)}.{$ext}">
    <xsl:value-of select="email/@name"/></a></td>
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
 <xsl:apply-templates select="server" mode="header"/>

 <br/>
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
          <a href="../mindex/{id}@{string(floor((number(messages) - 1) div 20)*20)}.{$ext}">
          <xsl:value-of select="email/@name"/></a></td>
        <xsl:if test="description"><td><xsl:value-of select="description"/></td></xsl:if>
        <xsl:if test="not(description)"><td>--</td></xsl:if>
       </tr>
      </xsl:if>
      <xsl:if test="position()=ceiling(last() div 2)">
       <tr class="list">
        <td class="center">
         <a href="../mindex/{id}@{string(floor((number(messages) - 1) div 20)*20)}.{$ext}">
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
         <a href="../mindex/{id}@{string(floor((number(messages) - 1) div 20)*20)}.{$ext}">
         <xsl:value-of select="email/@name"/></a></td>
        <xsl:if test="description"><td><xsl:value-of select="description"/></td></xsl:if>
        <xsl:if test="not(description)"><td>--</td></xsl:if>
       </tr>
      </xsl:if>
      <xsl:if test="position()=last()">
       <tr>
        <td class="center">
         <a href="../mindex/{id}@{string(floor((number(messages) - 1) div 20)*20)}.{$ext}">
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

 <div class="center">
 <form action ="../lurker-search.cgi">
  <input type="hidden" name="format" value="{$ext}"/>
  <input type="text"   name="query"  value="" size="50"/>
  <input type="submit" name="submit" value="{$search}!"/>
 <hr class="search"/>
  <table class="search">
   <tr><td><xsl:copy-of select="$author"/></td>
       <td class="center"><input type="text" name="author"  size="53" value=""/></td></tr>
   <tr><td><xsl:copy-of select="$subject"/></td>
       <td class="center"><input type="text" name="subject" size="53" value=""/></td></tr>
   <tr><td><xsl:copy-of select="$date"/></td>
    <td class="center">
     <select name="weekday">
       <option value=""><xsl:value-of select="$all-da"/></option>
       <option value="sun"><xsl:value-of select="$sun"/></option>
       <option value="mon"><xsl:value-of select="$mon"/></option>
       <option value="tue"><xsl:value-of select="$tue"/></option>
       <option value="wed"><xsl:value-of select="$wed"/></option>
       <option value="thu"><xsl:value-of select="$thu"/></option>
       <option value="fri"><xsl:value-of select="$fri"/></option>
       <option value="sat"><xsl:value-of select="$sat"/></option>
     </select>
   <xsl:value-of select="$join1"/>
     <select name="dom">
       <option value="">1-31</option>
       <option value="1">1</option>
       <option value="2">2</option>
       <option value="3">3</option>
       <option value="4">4</option>
       <option value="5">5</option>
       <option value="6">6</option>
       <option value="7">7</option>
       <option value="8">8</option>
       <option value="9">9</option>
       <option value="10">10</option>
       <option value="11">11</option>
       <option value="12">12</option>
       <option value="13">13</option>
       <option value="14">14</option>
       <option value="15">15</option>
       <option value="16">16</option>
       <option value="17">17</option>
       <option value="18">18</option>
       <option value="19">19</option>
       <option value="20">20</option>
       <option value="21">21</option>
       <option value="22">22</option>
       <option value="23">23</option>
       <option value="24">24</option>
       <option value="25">25</option>
       <option value="26">26</option>
       <option value="27">27</option>
       <option value="28">28</option>
       <option value="29">29</option>
       <option value="30">30</option>
       <option value="31">31</option>
     </select>
   <xsl:value-of select="$join2"/>
     <select name="month">
       <option value=""><xsl:value-of select="$all-mo"/></option>
       <option value="jan"><xsl:value-of select="$jan"/></option>
       <option value="feb"><xsl:value-of select="$feb"/></option>
       <option value="mar"><xsl:value-of select="$mar"/></option>
       <option value="apr"><xsl:value-of select="$apr"/></option>
       <option value="may"><xsl:value-of select="$may"/></option>
       <option value="jun"><xsl:value-of select="$jun"/></option>
       <option value="jul"><xsl:value-of select="$jul"/></option>
       <option value="aug"><xsl:value-of select="$aug"/></option>
       <option value="sep"><xsl:value-of select="$sep"/></option>
       <option value="oct"><xsl:value-of select="$oct"/></option>
       <option value="nov"><xsl:value-of select="$nov"/></option>
       <option value="dec"><xsl:value-of select="$dec"/></option>
     </select>
   /
     <select name="year">
       <option value=""><xsl:value-of select="$all-ye"/></option>
       <option value="2002">2002</option>
       <option value="2001">2001</option>
       <option value="2000">2000</option>
       <option value="1999">1999</option>
       <option value="1998">1998</option>
       <option value="1997">1997</option>
       <option value="1996">1996</option>
       <option value="1995">1995</option>
       <option value="1994">1994</option>
       <option value="1993">1993</option>
       <option value="1992">1992</option>
       <option value="1991">1991</option>
       <option value="1990">1990</option>
       <option value="1989">1989</option>
       <option value="1988">1988</option>
       <option value="1987">1987</option>
       <option value="1986">1986</option>
       <option value="1985">1985</option>
       <option value="1984">1984</option>
       <option value="1983">1983</option>
       <option value="1982">1982</option>
       <option value="1981">1981</option>
       <option value="1980">1980</option>
     </select>
    </td>
   </tr>
   <tr>
    <td><xsl:copy-of select="$appearin"/></td>
    <td class="center">
     <select name="list">
      <option value=""><xsl:value-of select="$all-li"/></option>
      <xsl:apply-templates mode="select" select="list"/>
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
  <b><xsl:value-of select="email/@name"/></b>
  <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;</xsl:text>
  <xsl:if test="email/@address">
   <a href="mailto:{email/@address}">(<xsl:value-of select="email/@address"/>)</a>
  </xsl:if>
 </dt>
 <dd>[<xsl:value-of select="messages"/><xsl:value-of select="$mess"/>]
     <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;--&amp;nbsp;&amp;nbsp;</xsl:text>
     <I><xsl:value-of select="description"/></I></dd>
</xsl:template>

</xsl:stylesheet>
