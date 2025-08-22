<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8" />
  </head>
  <body>
    <?php
      mb_language("Japanese");
      mb_internal_encoding("UTF-8");
      $to = $_POST['to'];
      $title = $_POST['title'];
      $content = $_POST['content'];
      $content = str_replace("\r\n", "\n", $content);// 改行コードの修正

      $option= "From:".mb_encode_mimeheader("小高 PC")."<yoshiyuki.odaka@dc.kyocera.com>";

      if(mb_send_mail($to, $title, $content, $option)){
        echo "メールを送信しました";
      } else {
        echo "メールの送信に失敗しました";
      };
    ?>
  </body>
</html>


