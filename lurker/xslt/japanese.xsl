<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    version="1.0">
 
 <xsl:variable name="lang">jp</xsl:variable>
 <xsl:variable name="adminby">管理者: </xsl:variable>
 <xsl:variable name="error">あなたのリクエストに対してエラーが発生しま した</xsl:variable>
 <xsl:variable name="redirect">リダイレクト</xsl:variable>
 <xsl:variable name="mredirect">ブラウザーはこの場所をロードするべきです:</xsl:variable>
 
 <xsl:variable name="someone">Someone</xsl:variable>
 <xsl:variable name="post">'s post on</xsl:variable>
 
 <xsl:variable name="subject">サブジェクト</xsl:variable>
 <xsl:variable name="date">日付</xsl:variable>
 <xsl:variable name="author">送信者</xsl:variable>
 <xsl:variable name="to">受信者</xsl:variable>
 <xsl:variable name="cc">CC</xsl:variable>
 <xsl:variable name="irt">返信</xsl:variable>
 <xsl:variable name="fus">フォローアップ</xsl:variable>
 <xsl:variable name="appearinmbox">メーリングリスト</xsl:variable>
 <xsl:variable name="mailbox">メールボックス</xsl:variable>
 <xsl:variable name="mailboxof">の</xsl:variable>
 <xsl:variable name="reply">返信</xsl:variable>
 
 <xsl:variable name="thedate"></xsl:variable>
 <xsl:variable name="ofdate">/</xsl:variable>
 <xsl:variable name="datedivide">/</xsl:variable>

 <xsl:variable name="appearinlist">メーリングリスト</xsl:variable>
 <xsl:variable name="mess">メッセージ</xsl:variable>
 <xsl:variable name="message">メッセージ</xsl:variable>
 <xsl:variable name="threads">新トピック</xsl:variable>

 <xsl:variable name="splash">フロント ページ</xsl:variable>
 <xsl:variable name="threading">ディスカッション</xsl:variable>
 <xsl:variable name="lin-h">メーリングリスト</xsl:variable>
 <xsl:variable name="des-h">説明</xsl:variable>
 <xsl:variable name="search">検索</xsl:variable>

 <xsl:variable name="all-da">日-土</xsl:variable>
 <xsl:variable name="sun">日</xsl:variable>
 <xsl:variable name="mon">月</xsl:variable>
 <xsl:variable name="tue">火</xsl:variable>
 <xsl:variable name="wed">水</xsl:variable>
 <xsl:variable name="thu">木</xsl:variable>
 <xsl:variable name="fri">金</xsl:variable>
 <xsl:variable name="sat">土</xsl:variable>

 <xsl:variable name="all-mo">すべての月</xsl:variable>
 <xsl:variable name="jan">１月</xsl:variable>
 <xsl:variable name="feb">２月</xsl:variable>
 <xsl:variable name="mar">３月</xsl:variable>
 <xsl:variable name="apr">４月</xsl:variable>
 <xsl:variable name="may">５月</xsl:variable>
 <xsl:variable name="jun">６月</xsl:variable>
 <xsl:variable name="jul">７月</xsl:variable>
 <xsl:variable name="aug">８月</xsl:variable>
 <xsl:variable name="sep">９月</xsl:variable>
 <xsl:variable name="oct">１０月</xsl:variable>
 <xsl:variable name="nov">１１月</xsl:variable>
 <xsl:variable name="dec">１２月</xsl:variable>

 <xsl:variable name="all-ye">すべての年</xsl:variable>
 <xsl:variable name="all-li">すべてのリスト</xsl:variable>
 <xsl:variable name="info">メーリングリスト情報</xsl:variable>
</xsl:stylesheet>
