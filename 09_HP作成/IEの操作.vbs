Dim ie  ' IE�p�ϐ�
    Set ie = CreateObject("InternetExplorer.Application")   ' IE�N��
    ie.Navigate "http://enginesd.php.xdomain.jp/main.php"  ' URL
    ie.Visible = False   ' IE�̉���
'    waitIE ie   ' IE�̋N���ҋ@



' �����j��
     ie.Quit ' IE�I��
