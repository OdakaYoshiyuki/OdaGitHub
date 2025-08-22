<!DOCTYPE html>
<html>
	<head>
		<meta charset="utf-8" />
	</head>

<body>
    <?php
	$title = $_POST['title'];
	$bot1 = $_POST['send1'];
	$bot2 = $_POST['send2'];
	$bot3 = $_POST['send3'];
	$bot4 = $_POST['send4'];
	$ki = intval($_POST['gradeName']); //文字列をintに変換

//参加者ファイル読み込み
	$sankaNum = 0;
	$file = fopen("sanka.txt", "r"); 
	if($file){
		while ($line = fgets($file)) {
		$inputSankaGrade[$sankaNum] = intval($line);
		if ($line = fgets($file)) {
			$inputSankaMenber[$sankaNum] = $line;
		}
		$sankaNum++;
		}
	}
	fclose($file);
//不参加者ファイル読み込み
	$husankaNum = 0;
	$file = fopen("husanka.txt", "r"); 
	if($file){ 
		while ($line = fgets($file)) {
		$inputHusankaGrade[$husankaNum] = intval($line);
		if ($line = fgets($file)) {
			$inputHusankaMenber[$husankaNum] = $line;
		}
		$husankaNum++;
		}
	}
	fclose($file);


	if($bot1 == "すべて削除"){
		$sankaGrade[0] = 0;
		$sankaMenber[0] = "start"."\n";
		$sankaGrade[1] = 1000;
		$sankaMenber[1] = "end"."\n";
		$sankaNum = 2;

		$husankaGrade[0] = 0;
		$husankaMenber[0] = "start"."\n";
		$husankaGrade[1] = 1000;
		$husankaMenber[1] = "end"."\n";
		$husankaNum = 2;

		$miteiGrade[0] = 0;
		$miteiMenber[0] = "start"."\n";
		$miteiGrade[1] = 1000;
		$miteiMenber[1] = "end"."\n";
		$miteiNum = 2;
	} else if ($bot1 == "出席") {
		$j = 0;
		for ($i = 0; $i < $sankaNum; $i++) {
			if ($ki <= $inputSankaGrade[$i]) {
			if ($ki > $inputSankaGrade[$i-1]) {
				$sankaGrade[$j] = $ki;
				$sankaMenber[$j] = $title."\n";
			$j++;
			}
		}
		if (($inputSankaGrade[$i] != $ki) || ($inputSankaMenber[$i] != ($title."\n"))) {
	            $sankaGrade[$j] = $inputSankaGrade[$i];
	            $sankaMenber[$j] = $inputSankaMenber[$i];
	            $j++;
		}
	    }
	    $sankaNum = $j;
	    $j = 0;
	    for ($i = 0; $i < $husankaNum; $i++) {
		if (($inputHusankaGrade[$i] != $ki) || ($inputHusankaMenber[$i] != ($title."\n"))) {
		    $husankaGrade[$j] = $inputHusankaGrade[$i];
		    $husankaMenber[$j] = $inputHusankaMenber[$i];
		    $j++;
		}
	    }
	    $husankaNum = $j;
	}

	if ($bot2 == "欠席") {
	    $j = 0;
	    for ($i = 0; $i < $husankaNum; $i++) {
	        if ($ki <= $inputHusankaGrade[$i]) {
		    if ($ki > $inputSankaGrade[$i-1]) {
		        $husankaGrade[$j] = $ki;
		        $husankaMenber[$j] = $title."\n";
			$j++;
		    }
	        }
		if (($inputHusankaGrade[$i] != $ki) || ($inputHusankaMenber[$i] != ($title."\n"))) {
	            $husankaGrade[$j] = $inputHusankaGrade[$i];
	            $husankaMenber[$j] = $inputHusankaMenber[$i];
	            $j++;
		}
	    }
	    $husankaNum = $j;
	    $j = 0;
	    for ($i = 0; $i < $sankaNum; $i++) {
		if (($inputSankaGrade[$i] != $ki) || ($inputSankaMenber[$i] != ($title."\n"))) {
		    $sankaGrade[$j] = $inputSankaGrade[$i];
		    $sankaMenber[$j] = $inputSankaMenber[$i];
		    $j++;
		}
	    }
	    $sankaNum = $j;
	}
	if ($bot3 == "削除") {
	    $j = 0;
	    for ($i = 0; $i < $sankaNum; $i++) {
		if (($inputSankaGrade[$i] != $ki) || ($inputSankaMenber[$i] != ($title."\n"))) {
		    $sankaGrade[$j] = $inputSankaGrade[$i];
		    $sankaMenber[$j] = $inputSankaMenber[$i];
		    $j++;
		}
	    }
	    $sankaNum = $j;
	    $j = 0;
	    for ($i = 0; $i < $husankaNum; $i++) {
		if (($inputHusankaGrade[$i] != $ki) || ($inputHusankaMenber[$i] != ($title."\n"))) {
		    $husankaGrade[$j] = $inputHusankaGrade[$i];
		    $husankaMenber[$j] = $inputHusankaMenber[$i];
		    $j++;
		}
	    }
	    $husankaNum = $j;
	}
	if ($bot4 == "確認") {
	    for ($i = 0; $i < $sankaNum; $i++) {
		$sankaGrade[$i] = $inputSankaGrade[$i];
		$sankaMenber[$i] = $inputSankaMenber[$i];
	    }
	    for ($i = 0; $i < $husankaNum; $i++) {
		$husankaGrade[$i] = $inputHusankaGrade[$i];
		$husankaMenber[$i] = $inputHusankaMenber[$i];
	    }
	}

//------------ファイルに書き込み------------
	$FP = fopen("sanka.txt","w"); //w書き込み、r読み込み、a追記
	//ファイルに書き込み
	for ($i = 0; $i < $sankaNum; $i++) {
	    fwrite($FP,$sankaGrade[$i]."\n");
	    fwrite($FP,$sankaMenber[$i]);
	}
	fclose($FP);
	$FP = fopen("husanka.txt","w"); //w書き込み、r読み込み、a追記
	//ファイルに書き込み
	for ($i = 0; $i < $husankaNum; $i++) {
	    fwrite($FP,$husankaGrade[$i]."\n");
	    fwrite($FP,$husankaMenber[$i]);
	}
	fclose($FP);

//------------表示------------
echo '<br>';
echo("[出席]");
echo '<br>';
	for ($i = 1; $i < $sankaNum-1; $i++) {
		if ($sankaGrade[$i] == 1) {
			echo("顧問、コーチ　");
		} else {
		echo($sankaGrade[$i]);
		echo("期 ");
		}
		echo($sankaMenber[$i]); 
		echo '<br>';
	}
echo '<br>';
echo("[欠席]");
echo '<br>';
	for ($i = 1; $i < $husankaNum-1; $i++) {
		if ($husankaGrade[$i] == 1) {
			echo("顧問、コーチ　");
		} else {
			echo($husankaGrade[$i]);
			echo("期 ");
		}
		echo($husankaMenber[$i]); 
		echo '<br>';
	}
echo '<br>';




    ?>
</body>
</html>






