Dim ie  ' IE用変数
    Set ie = CreateObject("InternetExplorer.Application")   ' IE起動
    ie.Navigate "http://enginesd.php.xdomain.jp/main.php"  ' URL
    ie.Visible = False   ' IEの可視化
'    waitIE ie   ' IEの起動待機



' 制御を破棄
     ie.Quit ' IE終了
