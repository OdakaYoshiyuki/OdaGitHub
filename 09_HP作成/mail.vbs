Option Explicit

    Dim objIE
    Dim objLink

    Set objIE = CreateObject("InternetExplorer.Application")
    objIE.Visible = True

    'IE���J��
    objIE.navigate "http://enginesd.php.xdomain.jp/confirm.html"
    
    '�y�[�W���ǂݍ��܂��܂ő҂�
    Do While objIE.Busy = True Or objIE.readyState <> 4
        WScript.Sleep 100        
    Loop


    objIE.document.getElementsByName("to")(0).Value = "y_odaka@hotmail.com"
    objIE.document.getElementsByName("title")(0).Value = "����"
    objIE.document.getElementsByName("content")(0).Value = "����������"

    '���M
    objIE.document.forms(0).submit()

    objIE.Quit
    Set objIE = Nothing
