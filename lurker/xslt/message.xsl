<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">

<!-- Format the navigation graphics -->

<xsl:template match="prev"><xsl:call-template name="msg-thread"/></xsl:template>
<xsl:template match="next"><xsl:call-template name="msg-thread"/></xsl:template>

<xsl:template name="navigate-fields">
 <div align="right">

  <!-- Draw the prev link - otherwise, draw an image with the same size -->
  <xsl:if test="threading/prev">
   <xsl:apply-templates select="threading/prev"/>
  </xsl:if>
  <xsl:if test="not(threading/prev)">
   <img src="../imgs/a.png" alt="."/>
  </xsl:if>

  <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;</xsl:text>

  <xsl:if test="reply">
   <a href="mailto:{reply}">(<xsl:value-of select="$reply"/>)</a>
  </xsl:if>
  <xsl:if test="not(reply)">
   <span class="na">(<xsl:value-of select="$reply"/>)</span>
  </xsl:if>

  <xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;nbsp;</xsl:text>

  <!-- Draw the next link - otherwise, draw an image with the same size -->
  <xsl:if test="threading/next">
   <xsl:apply-templates select="threading/next"/>
  </xsl:if>
  <xsl:if test="not(threading/next)">
   <img src="../imgs/a.png" alt="."/>
  </xsl:if>
 </div>
 <hr id="snippit"/>
 <xsl:apply-templates select="threading/snippet"/>
</xsl:template>


<!-- Format mailing list information -->

<xsl:template name="list-format">
 <a href="../mindex/{string(floor(number(offset) div 20)*20)}@{id}.{$ext}#{../id}">
  <xsl:value-of select="email/@name"/><xsl:text disable-output-escaping="yes">&amp;nbsp;&amp;#8722;&amp;nbsp;</xsl:text><xsl:value-of select="$message"/><xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>#<xsl:value-of select="offset"/>
 </a>
</xsl:template>

<xsl:template match="list" mode="message">
 <xsl:if test="position()!=1">, </xsl:if><xsl:call-template name="list-format"/>
</xsl:template>


<!-- Format headers for the message -->

<xsl:template name="header-fields">
 <div id="js">
  <form>
   <u><xsl:value-of select="$headdet"/></u>
   <div id="radio">
    <input type="radio" disabled="1" name="min" title="{$minimum}"/>
    <xsl:text disable-output-escaping="yes"> &amp;#8211; </xsl:text>
    <input type="radio" disabled="1" name="norm" title="{$normal}"/>
    <xsl:text disable-output-escaping="yes"> &amp;#8211; </xsl:text>
    <input type="radio" disabled="1" name="full" title="{$maximum}"/>
   </div>
   <div id="label">
    <xsl:value-of select="$less"/>
    <xsl:text disable-output-escaping="yes"> &amp;#8212;&amp;#8211 </xsl:text>
    <xsl:value-of select="$more"/>
   </div>
   <span>(<xsl:value-of select="$javas"/>)</span>
  </form>
 </div>
 <table id="h-fields">
  <tr>
   <th nowrap="1"><xsl:value-of select="$author"/>:</th>
   <td>
    <xsl:if test="email"><xsl:apply-templates select="email" mode="list"/></xsl:if>
    <xsl:if test="not(email)"><xsl:value-of select="$unknown"/></xsl:if>
   </td>
  </tr>
  <tr>
   <th nowrap="1"><xsl:value-of select="$date"/>:</th>
   <td>
    <xsl:if test="time"><xsl:value-of select="time"/></xsl:if>
    <xsl:if test="not(time)"><xsl:value-of select="$unknown"/></xsl:if>
   </td>
  </tr>
  <tr>
   <th nowrap="1"><xsl:value-of select="$to"/>:</th>
   <td>
    <xsl:if test="to"><xsl:apply-templates mode="list" select="to"/></xsl:if>
    <xsl:if test="not(to)"><xsl:value-of select="$unknown"/></xsl:if>
   </td>
  </tr>
  <xsl:if test="/message/cc">
   <tr><th nowrap="1"><xsl:value-of select="$cc"/>:</th> <td><xsl:apply-templates mode="list" select="cc"/></td></tr>
  </xsl:if>
  <xsl:if test="threading/inreplyto">
   <tr><th nowrap="1"><xsl:value-of select="$irt"/>:</th> <td><xsl:apply-templates mode="list" select="threading/inreplyto/summary"/></td></tr>
  </xsl:if>
  <xsl:if test="threading/replies">
   <tr><th nowrap="1"><xsl:value-of select="$fus"/>:</th> <td><xsl:apply-templates mode="list" select="threading/replies/summary"/></td></tr>
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

<xsl:template match="mime">
 <div class="mime-label">
  <xsl:if test="@name">
   <xsl:if test="ancestor::mime">--</xsl:if>
   <a href="../attach/{@id}@{/message/mid}@{@name}"><xsl:value-of select="@name"/> (<xsl:value-of select="@type"/>)</a>
   <xsl:if test="ancestor::mime">--</xsl:if>
  </xsl:if>
  <xsl:if test="not(@name)">
   <xsl:if test="ancestor::mime">--</xsl:if>
   <a href="../attach/{@id}@{/message/mid}.attach">(<xsl:value-of select="@type"/>)</a>
   <xsl:if test="ancestor::mime">--</xsl:if>
  </xsl:if>
 </div>
 <div class="mime"><xsl:apply-templates/></div>
 <xsl:if test="substring(@type,1,5) = string('image')">
  <img src="../attach/{@id}@{/message/mid}.{substring(@type,7,5)}"/>
 </xsl:if>
</xsl:template>

<!-- The main formatting procedure for a message -->


<xsl:template match="message" mode="title">
 <xsl:value-of select="list/email/@name"/> -
 <xsl:value-of select="subject"/>
</xsl:template>

<xsl:template match="message" mode="body">
 <h2>
  <a href="../thread/{thread}.{$ext}#{id}">
   <img src="../imgs/tree.png" alt="{$threading}:"/>
   <xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>
   <xsl:value-of select="subject"/>
  </a>
 </h2>

 <xsl:if test="threading/snippet/@cols &gt; 8">
  <xsl:call-template name="navigate-fields"/>
  <hr/>
  <xsl:call-template name="header-fields"/>
 </xsl:if>
 <xsl:if test="not(threading/snippet/@cols &gt; 8)">
  <table width="100%">
   <tr>
    <td width="100%"><xsl:call-template name="header-fields"/></td>
    <td id="navlet" nowrap="1"><xsl:call-template name="navigate-fields"/></td>
   </tr>
  </table>
 </xsl:if>

 <hr/>
 <div id="boxlist">
  <b>
   <xsl:value-of select="$appearinmbox"/>
   <xsl:text> </xsl:text>
   <a href="../mbox/{mid}.txt"><xsl:value-of select="$mailbox"/></a>
   <xsl:text> </xsl:text>
   <xsl:value-of select="$mailboxof"/>:
  </b>
   <xsl:text> </xsl:text>
   <xsl:apply-templates select="list" mode="message"/>
 </div>
 <hr/>

 <blockquote>
  <xsl:apply-templates select="mime"/>
 </blockquote>
</xsl:template>

</xsl:stylesheet>