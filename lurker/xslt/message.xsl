<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

<!-- Format the navigation graphics -->

<xsl:template name="navigate-fields">
 <div align="right">

  <!-- Draw the prev link - otherwise, draw an image with the same size -->
  <xsl:choose>
   <xsl:when test="threading/prev">
    <xsl:call-template name="msg-img">
     <xsl:with-param name="alt" select="'&lt;-'"/>
     <xsl:with-param name="img" select="'prev'"/>
     <xsl:with-param name="class" select="'normal'"/>
     <xsl:with-param name="sum" select="threading/prev/summary"/>
    </xsl:call-template>
   </xsl:when>
   <xsl:otherwise>
    <img src="../imgs/a.png" alt=".."/>
   </xsl:otherwise>
  </xsl:choose>

  &#160;&#160;
  <a href="../thread/{summary/id}.{$ext}">(tree)</a>
  &#160;&#160;

  <!-- Draw the next link - otherwise, draw an image with the same size -->
  <xsl:choose>
   <xsl:when test="threading/next">
    <xsl:call-template name="msg-img">
     <xsl:with-param name="alt" select="'-&gt;'"/>
     <xsl:with-param name="img" select="'next'"/>
     <xsl:with-param name="class" select="'normal'"/>
     <xsl:with-param name="sum" select="threading/next/summary"/>
    </xsl:call-template>
   </xsl:when>
   <xsl:otherwise>
    <img src="../imgs/a.png" alt=".."/>
   </xsl:otherwise>
  </xsl:choose>
 </div>
 
 <hr id="snippit"/>
 <xsl:for-each select="threading/snippet/tree">
  <xsl:apply-templates/><br/>
 </xsl:for-each>
</xsl:template>


<!-- Format mailing list information -->

<xsl:template match="mbox" mode="message">
 <xsl:if test="position() != 1">, </xsl:if>

 <a href="../mindex/{list/id}@{/message/summary/id}.{$ext}">
  <xsl:value-of select="list/email/@name"/>
 </a>
 <xsl:text>&#160;</xsl:text>

 <!-- Draw the prev link - otherwise, draw an image with the same size -->
 <xsl:choose>
  <xsl:when test="prev">
   <xsl:call-template name="msg-img">
    <xsl:with-param name="alt" select="'&lt;-'"/>
    <xsl:with-param name="img" select="'prev'"/>
    <xsl:with-param name="class" select="'normal'"/>
    <xsl:with-param name="sum" select="prev"/>
   </xsl:call-template>
  </xsl:when>
  <xsl:otherwise><img src="../imgs/a.png" alt=".."/></xsl:otherwise>
 </xsl:choose>
 
 <!-- Draw the next link - otherwise, draw an image with the same size -->
 <xsl:choose>
  <xsl:when test="next">
   <xsl:call-template name="msg-img">
    <xsl:with-param name="alt" select="'-&gt;'"/>
    <xsl:with-param name="img" select="'next'"/>
    <xsl:with-param name="class" select="'normal'"/>
    <xsl:with-param name="sum" select="next"/>
   </xsl:call-template>
  </xsl:when>
  <xsl:otherwise><img src="../imgs/a.png" alt=".."/></xsl:otherwise>
 </xsl:choose>
</xsl:template>

<!-- Format headers for the message -->

<xsl:template name="header-fields">
 <table id="h-fields">
  <tr>
   <th nowrap="NOWRAP"><xsl:value-of select="$author"/>:</th>
   <td>
    <xsl:apply-templates select="summary/email" mode="list"/>
   </td>
  </tr>
  <tr>
   <th nowrap="NOWRAP"><xsl:value-of select="$date"/>:</th>
   <td>
    <xsl:apply-templates select="summary" mode="date"/> UTC
   </td>
  </tr>
  <tr>
   <th nowrap="NOWRAP"><xsl:value-of select="$to"/>:</th>
   <td>
    <xsl:if test="to"><xsl:apply-templates mode="list" select="to"/></xsl:if>
    <xsl:if test="not(to)"><xsl:value-of select="$unknown"/></xsl:if>
   </td>
  </tr>
  <xsl:if test="/message/cc">
   <tr><th nowrap="NOWRAP"><xsl:value-of select="$cc"/>:</th> <td><xsl:apply-templates mode="list" select="cc"/></td></tr>
  </xsl:if>
  <xsl:if test="threading/inreplyto">
   <tr><th nowrap="NOWRAP"><xsl:value-of select="$irt"/>:</th> <td><xsl:apply-templates mode="list" select="threading/inreplyto/summary"/></td></tr>
  </xsl:if>
  <xsl:if test="threading/replies">
   <tr><th nowrap="NOWRAP"><xsl:value-of select="$fus"/>:</th> <td><xsl:apply-templates mode="list" select="threading/replies/summary"/></td></tr>
  </xsl:if>
  <xsl:if test="threading/drift">
   <tr><th nowrap="NOWRAP"><xsl:value-of select="$futs"/>:</th> <td><xsl:apply-templates mode="list" select="threading/drift/summary"/></td></tr>
  </xsl:if>
 </table>
</xsl:template>


<!-- Format email message contents -->

<xsl:template match="mailto">
 <xsl:element name="a">
  <xsl:attribute name="href">
   <xsl:text>mailto:</xsl:text>
   <xsl:call-template name="my-escape-uri">
    <xsl:with-param name="str" select="."/>
   </xsl:call-template>
  </xsl:attribute>
  <xsl:value-of select="."/>
 </xsl:element>
</xsl:template>

<xsl:template match="url">
 <xsl:choose>
  <xsl:when test="substring(., 1, 4) = 'www.'">
    <a class="url" href="http://{.}"><xsl:value-of select="."/></a>
  </xsl:when>
  <xsl:otherwise>
    <a class="url" href="{.}"><xsl:value-of select="."/></a>
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>

<xsl:template match="quote">
 <i class="quote"><xsl:apply-templates/></i>
</xsl:template>

<xsl:template match="art">
 <pre class="art"><xsl:apply-templates/></pre>
</xsl:template>


<!-- Format the mime components of the email -->

<xsl:template name="mime-url">
 <xsl:text>../attach/</xsl:text>
 <xsl:value-of select="@id"/>
 <xsl:text>@</xsl:text>
 
 <xsl:call-template name="my-escape-uri">
  <xsl:with-param name="str" select="/message/summary/id"/>
 </xsl:call-template>
 
 <xsl:choose>
  <xsl:when test="@name">
   <xsl:text>@</xsl:text>
   <xsl:value-of select="@name"/>
  </xsl:when>
  <xsl:otherwise>
   <xsl:text>.</xsl:text>
   <xsl:value-of select="substring-after(@type, '/')"/>
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>

<xsl:template match="mime">
 <div class="mime-label">
  <xsl:if test="ancestor::mime">--</xsl:if>
  
  <xsl:element name="a">
   <xsl:attribute name="href">
    <xsl:call-template name="mime-url"/>
   </xsl:attribute>
   
   <xsl:value-of select="@name"/> (<xsl:value-of select="@type"/>)
  </xsl:element>
  <xsl:if test="ancestor::mime">--</xsl:if>
 </div>
 
 <div class="mime"><xsl:apply-templates/></div>
 
 <xsl:if test="substring(@type,1,5) = string('image')">
  <xsl:element name="img">
   <xsl:attribute name="src">
    <xsl:call-template name="mime-url"/>
   </xsl:attribute>
  </xsl:element>
 </xsl:if>
</xsl:template>

<!-- The main formatting procedure for a message -->


<xsl:template match="message" mode="title">
 <xsl:value-of select="summary/subject"/>
</xsl:template>

<xsl:template match="message" mode="body">
 <h2>
  <a href="../thread/{summary/id}.{$ext}#{summary/id}">
   <img src="../imgs/tree.png" alt="{$threading}:"/>
   <xsl:text>&#160;</xsl:text>
   <xsl:value-of select="summary/subject"/>
  </a>
 </h2>

 <xsl:choose>
  <xsl:when test="threading/snippet/@cols &gt; 8">
   <xsl:call-template name="navigate-fields"/>
   <hr/>
   <xsl:call-template name="header-fields"/>
  </xsl:when>
  <xsl:otherwise>
   <table width="100%">
    <tr>
     <td width="100%"><xsl:call-template name="header-fields"/></td>
     <td id="navlet" nowrap="NOWRAP"><xsl:call-template name="navigate-fields"/></td>
    </tr>
   </table>
  </xsl:otherwise>
 </xsl:choose>

 <hr/>
 <div id="boxlist">
  <b>
   <xsl:value-of select="$appearinmbox"/>
   <xsl:text> </xsl:text>
   <a href="../mbox/{summary/id}.txt">
    <xsl:value-of select="$mailbox"/>
   </a>
   
   <xsl:text> </xsl:text>
   <xsl:value-of select="$mailboxof"/>:
  </b>
  <xsl:apply-templates select="mbox" mode="message"/>
 </div>
 <hr/>

 <blockquote>
  <xsl:apply-templates select="mime"/>
 </blockquote>
</xsl:template>

</xsl:stylesheet>
