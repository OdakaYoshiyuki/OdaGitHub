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
  // �T�[�o�ݒ�
  $mail->isSMTP();
  $mail->Host = 'smtp.gmail.com';        // ���[���T�[�o
  $mail->SMTPAuth = true;
  $mail->Username = 'y.odaka.519@gmail.com';  // ���M�A�J�E���g
  $mail->Password = 'a5krt6fe';           // ���M�A�J�E���g�̃p�X���[�h
  $mail->SMTPSecure = false;              // TLS�Ȃǂ̈Í�����Ή��̃T�[�o�Ȃ�false��ݒ�B�g����Ȃ�'tls'��'ssl'��ݒ�B
  $mail->SMTPAutoTLS = false;             // SMTPSecure��false�ɂ���ꍇ��false�ɂ���B����ȊO�Ȃ疢�ݒ�ŁB
  $mail->Port = 587;

    // �������
  $mail->setFrom('y.odaka.519@gmail.com');       // ���M���A�h���X
  $mail->addAddress('y_odaka@hotmai.com');  // ���M��A�h���X
  $mail->addReplyTo('y_odaka@hotmai.com');    // �ԐM��A�h���X

  // �{��
  $mail->isHTML(false);
  $mail->CharSet = 'UTF-8';

  $mail->Body = "�����ɖ{�����L��";
  $mail->Subject = "�����Ɍ������L��";

  $mail->send();
} catch (Exception $e) {
    $error = $e;
    echo $mail->ErrorInfo;
}
if ($error!=null) {
    echo "send";
}
