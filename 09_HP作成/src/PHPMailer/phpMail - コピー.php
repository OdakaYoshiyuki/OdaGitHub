<?php
use PHPMailer\PHPMailer\PHPMailer;
use PHPMailer\PHPMailer\Exception;

require 'PHPMailer/src/Exception.php';
require 'PHPMailer/src/PHPMailer.php';
require 'PHPMailer/src/SMTP.php';

$inquiry_no;
$mail = new PHPMailer(true);

$error = null;

try {
  // サーバ設定
  $mail->isSMTP();
  $mail->Host = 'smtp.gmail.com';        // メールサーバ
  $mail->SMTPAuth = true;
  $mail->Username = 'y.odaka.519@gmail.com';  // 送信アカウント
  $mail->Password = 'a5krt6fe';           // 送信アカウントのパスワード
  $mail->SMTPSecure = false;              // TLSなどの暗号化非対応のサーバならfalseを設定。使えるなら'tls'か'ssl'を設定。
  $mail->SMTPAutoTLS = false;             // SMTPSecureをfalseにする場合はfalseにする。それ以外なら未設定で。
  $mail->Port = 587;

    // 送り先情報
  $mail->setFrom('y.odaka.519@gmail.com');       // 送信元アドレス
  $mail->addAddress('y_odaka@hotmai.com');  // 送信先アドレス
  $mail->addReplyTo('y_odaka@hotmai.com');    // 返信先アドレス

  // 本文
  $mail->isHTML(false);
  $mail->CharSet = 'UTF-8';

  $mail->Body = "ここに本文を記載";
  $mail->Subject = "ここに件名を記載";

  $mail->send();
} catch (Exception $e) {
    $error = $e;
    echo $mail->ErrorInfo;
}
if ($error!=null) {
    echo "send";
}
