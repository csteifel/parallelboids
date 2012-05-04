<?php

	set_time_limit (600);
	
	$pixels_per_character = 20;
	$file = './concentric.jpg';
	list($width, $height) = getimagesize($file);

	$im = imagecreatefromjpeg($file);

	$output = "";		

	for($y = 0; $y < $height; $y++)
	{	
		for($x = 0; $x < $width; $x++)
		{
			$rgb = imagecolorat($im, $x, $y);
			$r = ($rgb >> 16) & 0xFF;
			$g = ($rgb >> 8) & 0xFF;
			$b = $rgb & 0xFF;

			if($r > 200 && $g > 200 && $b > 200)
			{
				$output .= " ";
			}
			else if($r > 200)
			{
				$output .= "w";
			}
			else if($g > 200)
			{
				$output .= "e";
			}
			else if($b > 200)
			{
				$output .= "b";
			}
		}
		$output .= "\n";
	}
	echo $output;
?>
