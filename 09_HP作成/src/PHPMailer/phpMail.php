

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
 // ���P�́A�Q�i�K�F�ؗL���Ȃ�u�A�v���ŗL�̃p�X���[�h�v�𐶐����Ă���������
 // �����Ȃ�u���S���̒Ⴂ�A�v������̃A�N�Z�X�����v���Ă���
$mail->From     = 'y.odaka.519@gmail.com'; 
$mail->FromName = mb_encode_mimeheader("y.odaka.519@gmail.com","ISO-2022-JP","UTF-8"); // "�\����" <���[���A�h���X>
$mail->Subject  = mb_encode_mimeheader("����","ISO-2022-JP", "UTF-8");
$mail->Body     = mb_convert_encoding("�{��","UTF-8","auto");
$mail->AddAddress('y_odaka@hotamil.com');

if (!$mail->send()) {
    echo "���M�G���[ " . $mail->ErrorInfo;
} else {
    echo "���M���܂���" . "\n";
}


?>



