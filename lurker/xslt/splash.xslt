<?xml version="1.0" encoding="ISO-8859-1" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"                    
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<xsl:output method="html" indent="yes" encoding="ISO-8859-1"
    doctype-system="http://www.w3.org/TR/html4/strict.dtd"
    doctype-public="-//W3C//DTD HTML 4.01//EN"/>

<xsl:template match="email[@address]">
  <a href="mailto:{@address}">
    <xsl:if test="@name">&quot;<xsl:value-of select="@name"/>&quot;</xsl:if>
    &lt;<xsl:value-of select="@address"/>&gt;
  </a>
</xsl:template>

<xsl:template match="email">
  <xsl:if test="not(@name)">
    SomeOne
  </xsl:if>
  <xsl:value-of select="@name"/>
</xsl:template>

<xsl:template match="list">
  <tr>
  <td><a href="../mindex/{id}@{string(floor(number(messages) div 20)*20)}.html">
        <xsl:value-of select="email/@name"/></a></td>

  <xsl:if test="email/@address">
    <td><a href="mailto:{email/@address}"><xsl:value-of select="email/@address"/></a></td>
  </xsl:if>
  <xsl:if test="not(email/@address)"><td>-</td></xsl:if>

  <xsl:if test="description"><td><xsl:value-of select="description"/></td></xsl:if>
  <xsl:if test="not(description)"><td>-</td></xsl:if>

  <td align="right"><xsl:value-of select="messages"/></td>
  </tr>
</xsl:template>

<!-- Document Root -->
<xsl:template match="/">
 <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
 <head>
  <title>
    Lurker@<xsl:value-of select="/lists/server/hostname"/> - Front Page
  </title>
 </head>
 <body>
  <h1>Lurker@<xsl:value-of select="/lists/server/hostname"/></h1>

  <table>

<tr><th align="left">List</th>
    <th align="left">Address</th>
    <th align="left">Description</th>
    <th align="center">Messages</th></tr>
    <xsl:apply-templates select="/lists/list"/>
  </table>
  <hr/>
  <h1>Lurker search</h1>
  <p><form action="../search/bounce">
    <input type="text" name="query" size="50" value=""/>
    <input type="submit" name="submit" value="Search!"/>
  <p><table>
      <tr><td>Author </td><td><input type="text" name="author" size="50" value=""/></td></tr>
      <tr><td>Subject</td><td><input type="text" name="subject" size="50" value=""/></td></tr>
      <tr><td>Date</td><td>
     <select name="weekday">
       <option value="">Sun-Sat</option>
       <option value="sun">Sunday</option>
       <option value="mon">Monday</option>
       <option value="tue">Tuesday</option>
       <option value="wed">Wednesday</option>
       <option value="thu">Sunday</option>
       <option value="fri">Friday</option>
       <option value="sat">Saturday</option>
     </select>, the
     <select name="dom">
       <option value="">1st-31st</option>
       <option value="1">1st</option>
       <option value="2">2nd</option>
       <option value="3">3rd</option>
       <option value="4">4th</option>
       <option value="5">5th</option>
       <option value="6">6th</option>
       <option value="7">7th</option>
       <option value="8">8th</option>
       <option value="9">9th</option>
       <option value="10">10th</option>
       <option value="11">11th</option>
       <option value="12">12th</option>
       <option value="13">13th</option>
       <option value="14">14th</option>
       <option value="15">15th</option>
       <option value="16">16th</option>
       <option value="17">17th</option>
       <option value="18">18th</option>
       <option value="19">19th</option>
       <option value="20">20th</option>
       <option value="21">21st</option>
       <option value="22">22nd</option>
       <option value="23">23rd</option>
       <option value="24">24th</option>
       <option value="25">25th</option>
       <option value="26">26th</option>
       <option value="27">27th</option>
       <option value="28">28th</option>
       <option value="29">29th</option>
       <option value="30">30th</option>
       <option value="31">31st</option>
     </select> of
     <select name="month">
       <option value="">All months</option>
       <option value="jan">January</option>
       <option value="feb">Feburary</option>
       <option value="mar">March</option>
       <option value="apr">April</option>
       <option value="may">May</option>
       <option value="jun">June</option>
       <option value="jul">July</option>
       <option value="aug">August</option>
       <option value="sep">September</option>
       <option value="oct">October</option>
       <option value="nov">November</option>
       <option value="dec">Decemember</option>
     </select> /
     <select name="year">
       <option value="">All years</option>
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
     </select></td></tr></table>
   </p>
  </form></p>
 <hr/>
 <p>Administrated by: <xsl:apply-templates select="/lists/server/email"/></p>
 </body>
 </html>
</xsl:template>

</xsl:stylesheet>
