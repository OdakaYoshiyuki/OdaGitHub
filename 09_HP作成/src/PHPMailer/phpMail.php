

<?php

use PHPMailer\PHPMailer\PHPMailer;
use PHPMailer\PHPMailer\Exception;

require 'src/Exception.php';
require 'src/PHPMailer.php';
require 'src/SMTP.php';

mb_language("japanese"); 
mb_internal_encoding("UTF-8");

$mail = new PHPMailer();
$mail->isSMTP();
$mail->Encoding = "7bit";
$mail->CharSet = '"UTF-8"';

$mail->Host = 'smtp.gmail.com';
$mail->Port = 587;
$mail->SMTPAuth = true;
$mail->SMTPSecure = "tls";
$mail->Username = 'y.odaka.519@gmail.com';
$mail->Password = 'a5krt6fe';
 // ※１は、２段階認証有効なら「アプリ固有のパスワード」を生成してそれをいれる
 // 無効なら「安全性の低いアプリからのアクセスを許可」しておく
$mail->From     = 'y.odaka.519@gmail.com'; 
$mail->FromName = mb_encode_mimeheader("y.odaka.519@gmail.com","ISO-2022-JP","UTF-8"); // "表示名" <メールアドレス>
$mail->Subject  = mb_encode_mimeheader("件名","ISO-2022-JP", "UTF-8");
$mail->Body     = mb_convert_encoding("本文","UTF-8","auto");
$mail->AddAddress('y_odaka@hotamil.com');

if (!$mail->send()) {
    echo "送信エラー " . $mail->ErrorInfo;
} else {
    echo "送信しました" . "\n";
}


?>



