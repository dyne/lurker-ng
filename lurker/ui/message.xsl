<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="common.xsl"/>

<!-- Format message links by subject -->
<xsl:template match="summary" mode="post-subject">
 <xsl:value-of select="subject"/>
</xsl:template>
<xsl:template match="summary" mode="post-subject-link">
 <a href="{id}.{$ext}">
  <xsl:apply-templates mode="post-subject" select="."/>
 </a>
</xsl:template>
<xsl:template match="summary" mode="post-subject-list">
 <xsl:if test="position()!=1">, </xsl:if>
 <xsl:apply-templates mode="post-subject-link" select="."/>
</xsl:template>


<!-- Format the message header fields -->
<xsl:template name="header-fields">
 <b><xsl:value-of select="$author"/>:&#160;</b>
 <xsl:apply-templates select="summary/email" mode="email-link"/><br/>
 <b><xsl:value-of select="$date"/>:&#160;</b>
 <xsl:apply-templates select="summary" mode="text-date"/>
 <xsl:text>&#160;</xsl:text>
 <xsl:apply-templates select="summary" mode="timezone"/>
 <br/>
 <xsl:if test="to">
  <b><xsl:value-of select="$to"/>:&#160;</b>
  <xsl:apply-templates mode="email-list" select="to/email"/><br/>
 </xsl:if>
 <xsl:if test="cc">
  <b><xsl:value-of select="$cc"/>:&#160;</b>
  <xsl:apply-templates mode="email-list" select="cc"/><br/>
 </xsl:if>
 <xsl:if test="threading/inreplyto">
  <b><xsl:value-of select="$old-topics"/>:&#160;</b>
  <xsl:apply-templates mode="post-subject-list" select="threading/inreplyto/summary"/><br/>
 </xsl:if>
 <xsl:if test="threading/drift">
  <b><xsl:value-of select="$new-topics"/>:&#160;</b>
  <xsl:apply-templates mode="post-subject-list" select="threading/drift/summary"/><br/>
 </xsl:if>
 <b><xsl:value-of select="$subject"/>:&#160;</b>
 <xsl:value-of select="summary/subject"/><br/>
</xsl:template>


<!-- Format the mime attachments -->
<xsl:template match="mime" mode="attach">
 <xsl:if test="translate(substring-before(@type,'/'),'MULTIPART','multipart')!='multipart'">
  <a href="../attach/{@id}@{/message/summary/id}.attach">
   <img src="../imgs/paperclip.png" alt="+"/>
   <xsl:if test="@name">
    <xsl:text>&#160;</xsl:text>
    <xsl:value-of select="@name"/>
   </xsl:if>
   <xsl:text>&#160;(</xsl:text>
   <xsl:value-of select="@type"/>
   <xsl:text>)</xsl:text>
  </a>
  <br/>
 </xsl:if>
 <xsl:apply-templates mode="attach" select="mime|signed"/>
</xsl:template>
<xsl:template match="signed" mode="attach">
 <xsl:apply-templates mode="attach" select="data/mime"/>
</xsl:template>
<xsl:template name="attachments">
 <table class="attachments">
  <tr><th align="left"><xsl:value-of select="$attachments"/></th></tr>
  <tr><td>
   <a href="../mbox/{summary/id}.txt"><xsl:value-of select="$raw-email"/></a><br/>
   <xsl:apply-templates mode="attach" select="mime"/>
  </td></tr>
 </table>
</xsl:template>


<!-- Format the message body -->
<xsl:template mode="body" match="mailto">
 <a class="email-address" href="mailto:{.}">
  <xsl:value-of select="."/>
 </a>
</xsl:template>
<xsl:template mode="body" match="url">
 <xsl:choose>
  <xsl:when test="substring(., 1, 4) = 'www.'">
    <a class="url" href="http://{.}"><xsl:value-of select="."/></a>
  </xsl:when>
  <xsl:otherwise>
    <a class="url" href="{.}"><xsl:value-of select="."/></a>
  </xsl:otherwise>
 </xsl:choose>
</xsl:template>
<xsl:template mode="body" match="quote">
 <i class="quote"><xsl:apply-templates mode="body"/></i>
</xsl:template>
<xsl:template mode="body" match="art">
 <br/><!-- browsers ignore the first br before a pre, so we need one more -->
 <pre class="art"><xsl:apply-templates mode="body"/></pre>
</xsl:template>
<xsl:template mode="body" match="br">
 <xsl:if test="not(ancestor::art)"><br/></xsl:if>
</xsl:template>
<xsl:template mode="body" match="signed">
 <xsl:element name="div">
  <xsl:attribute name="class">
   <xsl:choose>
    <xsl:when test="@ok='yes'">goodsig</xsl:when>
    <xsl:when test="@ok='unknown'">unknownsig</xsl:when>
    <xsl:otherwise>badsig</xsl:otherwise>
   </xsl:choose>
  </xsl:attribute>
  <xsl:if test="photo"><img src="{photo}" class="photo"/></xsl:if>
  <div class="details"><xsl:apply-templates mode="body" select="details"/></div>
  <div class="data"><xsl:apply-templates mode="body" select="data"/></div>
 </xsl:element>
</xsl:template>


<!-- Format the mailing lists -->
<xsl:template mode="appear-in" match="mbox">
 <tr>
  <td nowrap="NOWRAP" class="padded">
   <b><a name="{list/id}"/><xsl:value-of select="list/email/@name"/></b><br/>
   <a href="../list/{list/id}.{$ext}"><xsl:value-of select="$list-info"/></a> |
   <a href="../mindex/{list/id}@{../summary/id}.{$ext}"><xsl:value-of select="$near-message"/></a>
  </td>
  <xsl:choose>
   <xsl:when test="prev">
    <td>
     <a href="{prev/summary/id}.{$ext}#{list/id}">
      <img src="../imgs/prev.png" alt="&lt;-"/>
     </a>
    </td>
    <td align="left" width="50%">
     <a href="{prev/summary/id}.{$ext}#{list/id}">
      <xsl:value-of select="prev/summary/subject"/>
     </a>
    </td>
   </xsl:when>
   <xsl:otherwise><td>&#160;</td><td>&#160;</td></xsl:otherwise>
  </xsl:choose>
  <xsl:choose>
   <xsl:when test="next">
    <td align="right" width="50%">
     <a href="{next/summary/id}.{$ext}#{list/id}">
      <xsl:value-of select="next/summary/subject"/>
     </a>
    </td>
    <td>
     <a href="{next/summary/id}.{$ext}#{list/id}">
      <img src="../imgs/next.png" alt="-&gt;"/>
     </a>
    </td>
   </xsl:when>
   <xsl:otherwise><td>&#160;</td><td>&#160;</td></xsl:otherwise>
  </xsl:choose>
 </tr>
</xsl:template>


<!-- Format a message request -->
<xsl:template match="message">
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="../fmt/default.css" type="text/css"/>
   <title><xsl:value-of select="summary/subject"/></title>
   <script type="text/javascript" src="../fmt/common.js"/>
  </head>
  <body>
   <div class="header">
    <table class="external">
     <tr>
      <td align="left">
       <h1>
        <xsl:value-of select="substring(summary/subject,1,60)"/>
        <xsl:if test="string-length(summary/subject) &gt; 60">&#x2026;</xsl:if>
       </h1>
      </td>
      <td align="right">
       <h1><xsl:apply-templates mode="splash-link" select="server"/></h1>
      </td>
     </tr>
    </table>
    
    <table class="navigation">
     <tr><th align="left" colspan="2"><xsl:value-of select="$tree-context"/></th></tr>
     <tr>
      <td nowrap="NOWRAP">
       <xsl:apply-templates mode="tree" select="threading/snippet/tree[position()=1]"/>
      </td>
      <td align="right">
       <a href="../thread/{summary/id}.{$ext}#{summary/id}">
        <xsl:value-of select="$full-tree"/>
       </a>
      </td>
     </tr>
     <tr>
      <td nowrap="NOWRAP">
       <xsl:apply-templates mode="tree" select="threading/snippet/tree[position()=2]"/>
      </td>
      <td nowrap="NOWRAP" align="right">
       <xsl:choose>
        <xsl:when test="threading/prev">
         <a href="{threading/prev/summary/id}.{$ext}">
          <xsl:apply-templates mode="post-description-text" select="threading/prev/summary"/>
          <img src="../imgs/prev.png" alt="&lt;-"/>
         </a>
        </xsl:when>
        <xsl:otherwise>&#160;</xsl:otherwise>
       </xsl:choose>
      </td>
     </tr>
     <tr>
      <td nowrap="NOWRAP">
       <xsl:apply-templates mode="tree" select="threading/snippet/tree[position()=3]"/>
      </td>
      <td nowrap="NOWRAP" align="right">
       <xsl:choose>
        <xsl:when test="threading/next">
         <a href="{threading/next/summary/id}.{$ext}">
          <xsl:apply-templates mode="post-description-text" select="threading/next/summary"/>
          <img src="../imgs/next.png" alt="-&gt;"/>
         </a>
        </xsl:when>
        <xsl:otherwise>&#160;</xsl:otherwise>
       </xsl:choose>
      </td>
     </tr>
    </table>
   </div>
   
   <div class="body">
    <xsl:call-template name="attachments"/>
    <xsl:call-template name="header-fields"/>
    <div class="messageBody">
     <xsl:apply-templates mode="body" select="mime"/>
    </div>
   </div>
   
   <div class="footer">
    <table class="navigation">
     <tr>
      <th colspan="5" align="left">
       <xsl:value-of select="$mail-appears-in"/>
      </th>
     </tr>
     <xsl:apply-templates mode="appear-in" select="mbox"/>
    </table>
    <xsl:call-template name="lurker-signature"/>
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
