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


<!-- Format the reply link -->
<xsl:template match="email[@name]" mode="mailto">
  <xsl:if test="position()!=1">, </xsl:if>
  <xsl:text>&quot;</xsl:text>
  <xsl:value-of select="@name"/>
  <xsl:text>&quot; &lt;</xsl:text>
  <xsl:value-of select="@address"/>
  <xsl:text>&gt;</xsl:text>
</xsl:template>
<xsl:template match="email" mode="mailto">
  <xsl:if test="position()!=1">, </xsl:if>
  <xsl:value-of select="@address"/>
</xsl:template>

<xsl:template match="br" mode="mailtobody">
  <xsl:text>&gt; </xsl:text>
</xsl:template>
<xsl:template match="tab" mode="mailtobody">
  <xsl:text>    </xsl:text>
</xsl:template>
<xsl:template match="art" mode="mailtobody"/>
<xsl:template match="mime[@type='text/plain' or @type='text/html']" mode="mailtobody">
  <xsl:apply-templates mode="mailtobody"/>
</xsl:template>
<xsl:template match="mime" mode="mailtobody">
  <xsl:apply-templates select="mime|signed" mode="mailtobody"/>
</xsl:template>
<xsl:template match="signed" mode="mailtobody">
  <xsl:apply-templates select="data/mime" mode="mailtobody"/>
</xsl:template>

<xsl:template name="reply-link">
 <xsl:element name="a">
  <xsl:attribute name="href">
   <xsl:text>mailto:</xsl:text>
   <xsl:apply-templates select="mbox/list/email[@address]" mode="mailto"/>
   <xsl:text>?Subject=</xsl:text>
   <xsl:if test="not(contains(summary/subject, $regarding-subject))">
    <xsl:value-of select="$regarding-subject"/>
   </xsl:if>
   <xsl:value-of select="summary/subject"/>
   <xsl:if test="not(summary/subject)">your mail</xsl:if>
   <xsl:if test="message-id">
    <xsl:text>&amp;References=</xsl:text>
    <xsl:value-of select="message-id"/>
    <xsl:text>&amp;In-Reply-To=</xsl:text>
    <xsl:value-of select="message-id"/>
   </xsl:if>
   <xsl:variable name="listmails">
    <xsl:for-each select="mbox/list/email[@address]">
     <xsl:value-of select="translate(@address, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')"/>
     <xsl:text>,</xsl:text>
    </xsl:for-each>
   </xsl:variable>
   <xsl:variable name="allmails" select="(to/email[@address] | cc/email[@address] | summary/email[@address])"/>
   <xsl:variable name="restmails" select="$allmails[not(contains($listmails,translate(concat(@address,','), 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')))]"/>
   <xsl:if test="$restmails">
     <xsl:text>&amp;CC=</xsl:text>
     <xsl:apply-templates select="$restmails" mode="mailto"/>
   </xsl:if>
   <xsl:text>&amp;Body=</xsl:text>
   <xsl:value-of select="$quote-open"/>
   <xsl:apply-templates select="summary" mode="text-date"/>
   <xsl:value-of select="$quote-middle"/>
   <xsl:apply-templates select="summary/email" mode="email-name"/>
   <xsl:value-of select="$quote-close"/>
   <xsl:text>
&gt; </xsl:text>
   <xsl:apply-templates select="mime" mode="mailtobody"/>
  </xsl:attribute>
  <img src="../imgs/reply.png" alt="{$reply-to-message}" title="{$reply-to-message}"/>
 </xsl:element>
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
  <tr><th align="left"><xsl:value-of select="$attachments"/>:</th></tr>
  <tr><td>
   <a href="../mbox/{summary/id}.rfc822"><xsl:value-of select="$raw-email"/></a><br/>
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
<xsl:template mode="body" match="mime[@type='image/gif']">
  <a href="../attach/{@id}@{/message/summary/id}.gif">
   <img class="inline" src="../attach/{@id}@{/message/summary/id}.gif"/>
  </a>
  <br/>
  <xsl:apply-templates/>
</xsl:template>
<xsl:template mode="body" match="mime[@type='image/jpeg']">
  <a href="../attach/{@id}@{/message/summary/id}.jpg">
   <img class="inline" src="../attach/{@id}@{/message/summary/id}.jpg"/>
  </a>
  <br/>
  <xsl:apply-templates/>
</xsl:template>
<xsl:template mode="body" match="mime[@type='image/png']">
  <a href="../attach/{@id}@{/message/summary/id}.png">
    <img class="inline" src="../attach/{@id}@{/message/summary/id}.png"/>
  </a>
  <br/>
  <xsl:apply-templates/>
</xsl:template>
<xsl:template match="tab" mode="body">
  <xsl:text>&#160;&#160;&#160;&#160;</xsl:text>
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


<!-- Format the operation box on the right -->
<xsl:template name="opsbox">
 <table class="opsbox"><tr>
  <xsl:if test="/message/server/raw-email">
   <td><xsl:call-template name="attachments"/></td>
  </xsl:if>
  <td>
   <a href="javascript:trash('{server/doc-url}/zap/{summary/id}.{$ext}');">
    <img src="../imgs/trash.png" alt="{$delete-message}" title="{$delete-message}"/>
   </a>
   <xsl:if test="mbox/list/email/@address">
    <br/><xsl:call-template name="reply-link"/>
   </xsl:if>
  </td>
 </tr></table>
</xsl:template>


<!-- Format a message request -->
<xsl:template match="message">
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="../ui/default.css" type="text/css"/>
   <title><xsl:value-of select="summary/subject"/></title>
   <script type="text/javascript" src="../ui/common.js"/>
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
      <td align="right"><xsl:call-template name="navbar"/></td>
     </tr>
    </table>
    
    <table class="navigation">
     <tr><th align="left" colspan="2"><xsl:value-of select="$tree-context"/></th></tr>
     <tr>
      <td nowrap="NOWRAP">
       <xsl:apply-templates mode="tree" select="threading/snippet/tree[position()=1]"/>
      </td>
      <td align="right">
       <a href="../thread/{summary/id}.{$ext}#i{summary/id}">
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
    <xsl:choose>
     <!-- don't let the attachments run into the signature box -->
     <xsl:when test="//signed">
      <table class="opsbar"><tr>
       <td><xsl:call-template name="header-fields"/></td>
       <td align="right"><xsl:call-template name="opsbox"/></td>
      </tr></table>
     </xsl:when>
     <xsl:otherwise>
      <div class="opsfloat"><xsl:call-template name="opsbox"/></div>
      <div><xsl:call-template name="header-fields"/></div>
     </xsl:otherwise>
    </xsl:choose>
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
