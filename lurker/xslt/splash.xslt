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
  <td><a href="../mindex/{id}@{messages}"><xsl:value-of select="name"/></a></td>
  <td><xsl:value-of select="messages"/></td>
  <td><xsl:if test="address">
        <a href="mailto:{address}"><xsl:value-of select="address"/></a>
      </xsl:if></td>
  <td><xsl:value-of select="description"/></td>
  </tr>
</xsl:template>

<!-- Document Root -->
<xsl:template match="/">
 <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
 <head>
  <title>
    Lurker@some host
  </title>
 </head>
 <body>
  <h1>Lurker@some host
  </h1>

  <h2>Welcome</h2>

  <table>
    <xsl:apply-templates select="/lists/list"/>
  </table>
  <hr/>
  <h1>Lurker search</h1>
  <p><form action="../search/bounce">
    <input type="text" name="query" value=""/>
    <input type="submit" name="submit" value="Search!"/>
  <p><table>
      <tr><td>Author </td><td><input type="text" name="author" value=""/></td></tr>
      <tr><td>Subject</td><td><input type="text" name="author" value=""/></td></tr>
  </table></p>
  <hr/>
  <p><select name="weekday">
       <option value="">Sun-Sat</option>
       <option value="wd:sun">Sunday</option>
       <option value="wd:mon">Monday</option>
       <option value="wd:tue">Tuesday</option>
       <option value="wd:wed">Wednesday</option>
       <option value="wd:thu">Sunday</option>
       <option value="wd:fri">Friday</option>
       <option value="wd:sat">Saturday</option>
     </select>, the
     <select name="dom">
       <option value="">1st-31st</option>
       <option value="dm:1">1st</option>
       <option value="dm:2">2nd</option>
       <option value="dm:3">3rd</option>
       <option value="dm:4">4th</option>
       <option value="dm:5">5th</option>
       <option value="dm:6">6th</option>
       <option value="dm:7">7th</option>
       <option value="dm:8">8th</option>
       <option value="dm:9">9th</option>
       <option value="dm:10">10th</option>
       <option value="dm:11">11th</option>
       <option value="dm:12">12th</option>
       <option value="dm:13">13th</option>
       <option value="dm:14">14th</option>
       <option value="dm:15">15th</option>
       <option value="dm:16">16th</option>
       <option value="dm:17">17th</option>
       <option value="dm:18">18th</option>
       <option value="dm:19">19th</option>
       <option value="dm:20">20th</option>
       <option value="dm:21">21st</option>
       <option value="dm:22">22nd</option>
       <option value="dm:23">23rd</option>
       <option value="dm:24">24th</option>
       <option value="dm:25">25th</option>
       <option value="dm:26">26th</option>
       <option value="dm:27">27th</option>
       <option value="dm:28">28th</option>
       <option value="dm:29">29th</option>
       <option value="dm:30">30th</option>
       <option value="dm:31">31st</option>
     </select> of
     <select name="month">
       <option value="">All months</option>
       <option value="mn:jan">January</option>
       <option value="mn:feb">Feburary</option>
       <option value="mn:mar">March</option>
       <option value="mn:apr">April</option>
       <option value="mn:may">May</option>
       <option value="mn:jun">June</option>
       <option value="mn:jul">July</option>
       <option value="mn:aug">August</option>
       <option value="mn:sep">September</option>
       <option value="mn:oct">October</option>
       <option value="mn:nov">November</option>
       <option value="mn:dec">Decemember</option>
     </select> /
     <select name="year">
       <option value="">All years</option>
       <option value="yr:2002">2002</option>
       <option value="yr:2001">2001</option>
       <option value="yr:2000">2000</option>
       <option value="yr:1999">1999</option>
       <option value="yr:1998">1998</option>
       <option value="yr:1997">1997</option>
       <option value="yr:1996">1996</option>
       <option value="yr:1995">1995</option>
       <option value="yr:1994">1994</option>
       <option value="yr:1993">1993</option>
       <option value="yr:1992">1992</option>
       <option value="yr:1991">1991</option>
       <option value="yr:1990">1990</option>
       <option value="yr:1989">1989</option>
       <option value="yr:1988">1988</option>
       <option value="yr:1987">1987</option>
       <option value="yr:1986">1986</option>
       <option value="yr:1985">1985</option>
       <option value="yr:1984">1984</option>
       <option value="yr:1983">1983</option>
       <option value="yr:1982">1982</option>
       <option value="yr:1981">1981</option>
       <option value="yr:1980">1980</option>
     </select>
   </p>
  </form></p>
 </body>
 </html>
</xsl:template>

</xsl:stylesheet>
