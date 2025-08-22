<?php
 echo date("Y/m/d"); 

//言語と文字コードを設定
mb_language("Japanese"); 
mb_internal_encoding("UTF-8");

//メールの情報を設定
//$mailto = "y_odaka@hotmail.com,yoshiyuki.odaka@dc.kyocera.com";
//$mailto = "y_odaka@hotmail.com";
$mailto = "yoshiyuki.odaka@dc.kyocera.com";
$title = "タイトルテスト2";
$message = "本文のテスト。\n改行";
$option= "From:".mb_encode_mimeheader("odaka")."<yoshiyuki.odaka@dc.kyocera.com>";


//$header = "Content-Type:text/html;charset=UTF-8\r\n";
$header = "From:".mb_encode_mimeheader("小高PC")."<yoshiyuki.odaka@dc.kyocera.com>";

//$header  = "From: enginesd@sv5.php.xdomain.ne.jp\r\n";
$header .= "Return-Path: enginesd@sv5.php.xdomain.ne.jp\r\n";
//$header .= "Return-Path: y_odaka@hotmail.ne.jp\r\n";


//メールの送信
if(mb_send_mail($mailto,$title,$message,$header)){
	echo "送信成功";
}else{
	echo "送信失敗";
}

echo $message;
echo "本文のテストです。\n改行";


?>
