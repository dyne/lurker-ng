<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:import href="common.xsl"/>


<!-- Format the message header fields -->
<xsl:template name="header-fields">
 <table class="emailHeaders">
  <tr>
   <th nowrap="NOWRAP"><xsl:value-of select="$author"/>:</th>
   <td><xsl:apply-templates select="summary/email" mode="email-link"/></td>
  </tr>
  <tr>
   <th nowrap="NOWRAP"><xsl:value-of select="$date"/>:</th>
   <td><xsl:apply-templates select="summary" mode="date"/>&#160;UTC</td>
  </tr>
  <xsl:if test="to">
   <tr>
    <th nowrap="NOWRAP"><xsl:value-of select="$to"/>:</th>
    <td><xsl:apply-templates mode="email-list" select="to/email"/></td>
    <xsl:if test="not(to)"><xsl:value-of select="$unknown-address"/></xsl:if>
   </tr>
  </xsl:if>
  <xsl:if test="cc">
   <tr>
    <th nowrap="NOWRAP"><xsl:value-of select="$cc"/>:</th>
    <td><xsl:apply-templates mode="email-list" select="cc"/></td>
   </tr>
  </xsl:if>
  <xsl:if test="threading/inreplyto">
   <tr>
    <th nowrap="NOWRAP"><xsl:value-of select="$old-topics"/>:</th>
    <td><xsl:apply-templates mode="post-description-list" select="threading/inreplyto/summary"/></td>
   </tr>
  </xsl:if>
  <xsl:if test="threading/drift">
   <tr>
    <th nowrap="NOWRAP"><xsl:value-of select="$new-topics"/>:</th>
    <td><xsl:apply-templates mode="post-description-list" select="threading/drift/summary"/></td>
   </tr>
  </xsl:if>
 </table>
</xsl:template>


<!-- Format the mime attachments -->
<xsl:template match="mime" mode="attach">
 <xsl:if test="translate(substring-before(@type,'/'),'MULTIPART','multipart')!='multipart'">
  <a href="../attach/{@id}@{/message/summary/id}.attach">
   <img src="../imgs/paperclip.png" alt="+"/>
   <xsl:text> </xsl:text>
   <xsl:choose>
    <xsl:when test="@name"><xsl:value-of select="@name"/></xsl:when>
    <xsl:otherwise>unnamed</xsl:otherwise>
   </xsl:choose>
   <xsl:text> (</xsl:text>
   <xsl:value-of select="@type"/>
   <xsl:text>)</xsl:text>
  </a>
  <br/>
 </xsl:if>
 <xsl:apply-templates mode="attach" select="mime"/>
</xsl:template>
<xsl:template name="attachments">
 <table class="attachments">
  <tr><th align="left">Attachments</th></tr>
  <tr><td>
   <a href="../mbox/{summary/id}.txt"><xsl:value-of select="$raw-email"/></a><br/>
   <xsl:apply-templates mode="attach" select="mime"/>
  </td></tr>
 </table>
</xsl:template>


<!-- Format the message body -->
<xsl:template mode="body" match="mailto">
 <i class="email-address">
  <xsl:value-of select="."/>
 </i>
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
 <pre class="art"><xsl:apply-templates mode="body"/></pre>
</xsl:template>
<xsl:template mode="body" match="br">
 <xsl:if test="not(ancestor::art)"><br/></xsl:if>
</xsl:template>


<!-- Format the mailing lists -->
<xsl:template mode="appear-in" match="mbox">
 <tr>
  <td nowrap="NOWRAP" class="padded">
   <b><a name="{list/id}"><xsl:value-of select="list/email/@name"/></a></b><br/>
   <a href="../list/{list/id}.{$ext}"><xsl:value-of select="$list-info"/></a> |
   <a href="../mindex/{list/id}@{../summary/id}.{$ext}"><xsl:value-of select="$near-message"/></a>
  </td>
  <td align="left" width="50%">
   <xsl:choose>
    <xsl:when test="prev">
     <a href="{prev/summary/id}.{$ext}#{list/id}">
      <img src="../imgs/prev.png" alt="&lt;-"/>
      <xsl:value-of select="prev/summary/subject"/>
     </a>
    </xsl:when>
    <xsl:otherwise>&#160;</xsl:otherwise>
   </xsl:choose>
  </td>
  <td align="right" width="50%">
   <xsl:choose>
    <xsl:when test="next">
     <a href="{next/summary/id}.{$ext}#{list/id}">
      <xsl:value-of select="next/summary/subject"/>
      <img src="../imgs/next.png" alt="&lt;-"/>
     </a>
    </xsl:when>
    <xsl:otherwise>&#160;</xsl:otherwise>
   </xsl:choose>
  </td>
 </tr>
</xsl:template>


<!-- Format a message request -->
<xsl:template match="message">
 <html lang="{$lang}">
  <head>
   <link rel="stylesheet" href="default.css" type="text/css"/>
   <title><xsl:value-of select="summary/subject"/></title>
  </head>
  <body>
   <div class="header">
    <table class="external">
     <tr>
      <td width="100%">
       <h1>
        <xsl:value-of select="summary/subject"/>
       </h1>
      </td>
      <td nowrap="NOWRAP">
       <h1>
        <xsl:apply-templates mode="splash-link" select="server"/>
       </h1>
      </td>
     </tr>
    </table>
    
    <table class="navigation">
     <tr>
      <th colspan="2">
       <xsl:value-of select="$tree-context"/>
      </th>
     </tr>
     <tr>
      <td nowrap="NOWRAP">
       <xsl:apply-templates mode="tree" select="threading/snippet/tree[position()=1]"/>
      </td>
      <td align="right">
       <a href="../thread/{summary/id}.{$ext}">
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
          <xsl:apply-templates mode="post-description" select="threading/prev/summary"/>
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
          <xsl:apply-templates mode="post-description" select="threading/next/summary"/>
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
    <table class="header">
     <tr>
      <td><xsl:call-template name="header-fields"/></td>
      <td align="right"><xsl:call-template name="attachments"/></td>
     </tr>
    </table>
    <div class="messageBody">
     <xsl:apply-templates mode="body" select="mime"/>
    </div>
   </div>
   
   <div class="footer">
    <table class="navigation">
     <tr>
      <th colspan="3" align="left">
       <xsl:value-of select="$appears-in"/>
      </th>
     </tr>
     <xsl:apply-templates mode="appear-in" select="mbox"/>
    </table>
    
    <table class="external">
     <tr>
      <td class="mini">
       <b><a href="{$lurker-url}">Lurker</a></b>
       (version <xsl:value-of select="server/version"/>)
      </td>
      <td class="mini" align="right">
       <xsl:value-of select="$admin-by"/>
       <xsl:apply-templates mode="email-link" select="server/email"/>
      </td>
     </tr>
    </table>
   </div>
  </body>
 </html>
</xsl:template>

</xsl:stylesheet>
