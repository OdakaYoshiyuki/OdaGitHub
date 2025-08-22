Option Explicit

    Dim objIE
    Dim objLink

    Set objIE = CreateObject("InternetExplorer.Application")
    objIE.Visible = True

    'IEを開く
    objIE.navigate "http://enginesd.php.xdomain.jp/confirm.html"
    
    'ページが読み込まれるまで待つ
    Do While objIE.Busy = True Or objIE.readyState <> 4
        WScript.Sleep 100        
    Loop


    objIE.document.getElementsByName("to")(0).Value = "y_odaka@hotmail.com"
    objIE.document.getElementsByName("title")(0).Value = "件名"
    objIE.document.getElementsByName("content")(0).Value = "あいうえお"

    '送信
    objIE.document.forms(0).submit()

    objIE.Quit
    Set objIE = Nothing
