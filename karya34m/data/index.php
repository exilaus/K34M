<html>
	<head>
		<title>CostyKarya CNC gcode tracer</title>
<script async src="//pagead2.googlesyndication.com/pagead/js/adsbygoogle.js"></script>
<script>
     (adsbygoogle = window.adsbygoogle || []).push({
          google_ad_client: "ca-pub-9111193570003924",
          enable_page_level_ads: true
     });
</script>
	</head>


<body>
    <br>
	<center style="font-size:12pt;font-family:arial">Butuh Jasa cutting acrylic,triplek,mdf,Aluminium Composite, hubungi kami di 083838461040 atau emailkan desain ke ryannining@gmail.com</center>
	<hr><table width=100%>
    <tr valign=top width=1000px><td width=200px><div>
        <input type='file' accept='.bmp,.jpg,.gif,.png' id=btfile>
        <br>Retrace Smoothness <input id=smooth size=2 value="0.7" class=saveit>
        <hr>DPI <input id="scale" size=5 value="72" class=saveit>
		<br>

    Material:<select id=material class=saveit>
			<option value="10,1,23">Laser Akrilik 1.5mm</option>
			<option value="8,1,25">Laser Akrilik 2mm</option>
			<option value="8,1,30">Laser Akrilik 2mm Warna</option>
			<option value="6,1,35">Laser Akrilik 3mm</option>
			<option value="4,1,45">Laser Akrilik 4mm</option>
			<option value="4,1,50">Laser Akrilik 4mm Warna</option>
			<option value="3,1,55">Laser Akrilik 5mm</option>
			<option value="3,1,60">Laser Akrilik 5mm Warna</option>
			<option value="2,1,65">Laser Akrilik 6mm</option>
			<option value="2,1,70">Laser Akrilik 6mm Warna</option>
			<option value="6,1,6">Laser Triplek 5mm</option>
			<option value="6,2,6">Laser Triplek 9mm</option>
			<option value="6,1,6">MDF 5mm</option>
			<option value="6,2,6,9.5">CNC MDF 9mm</option>
			<option value="10,4,25,4.5">CNC ACP 4mm</option>
		</select><br>
		GCODE Type : <select id=cmode  class=saveit><option value=1>Laser</option><option value=3>CNC</option><option value=2>Foam</option></select> <br>
		<table style="font-size: 10pt;font-family: arial;text-align:  right;background: yellow;">
		<tr><td colspan=2 style="background:lime">Tools Off-On
		<tr><td><input id=pup size=8 value="m3 s0" class=saveit>
		<td><input id=pdn size=12 value="m3 s160" class=saveit>
		<tr><td colspan=2 style="background:lime">Motion
		<tr><td>Travel <input id=trav size=2 value=150 class=saveit>
		<td>Feed <input id=feed size=2 value=6 class=saveit> 
		<tr><td>Repeat <input id=repeat class=saveit size=1 value=1>
      <tr><td colspan=2 style="background:lime">CNC
	  <tr><td>Zdown <input id=zdown size=1 value=1 class=saveit>
     <td>Tab Cut <input id=tabc value=0 size=3  class=saveit>
	  <tr><td>Flip X <input id=flipx type=checkbox>
	  <td>Rotate <input id=rotate type=checkbox>
 	</table>
	<button id=btrecode >Re-gcode</button>
    </div>
    <B>INIT</b><button  id=btcopy1>Copy</button><br>
    <textarea style="font-size: 10;width:100%;height:70px" id="icode" class=saveit>
;Init machine
;===============
M206 P80 S20 ;x backlash
M206 P84 S20 ;y backlash
M206 P88 S20 ;z backlash
;===============
    </textarea>
    <B>GCODE</b><button  id=btcopy1>Copy</button><br>
    <textarea style="font-size: 10;width:100%;height:100px" id="gcode" class=saveit>
    </textarea>
    <br><B>Preview GCODE</b><button id=btcopy2>Copy</button><br>
    <textarea style="font-size: 10;width:100%;height:70px" id="pgcode" class=saveit>
    </textarea>
	<td wdith=480px><p id="area_dimension"></p>
    <canvas id="myCanvas1" width="480" height="500" style="border:1px solid #FF0000;"></canvas>
    <br><button id=btsaveset >Save State</button>

    <td width=300px><b>Machine Control</b><br>
	Serial Port <button id=btinitser>?</button> <select id=comport></select><br><button id=btconnect>Connect</button><br>
	Status:not connected
	<hr>

	<table><tr align=center valign=center>
	<td>Move:<input id=move value=50 size=1>mm
	<td><button id=btleft>&lt;</button>
	<td><button  id=btup>/\</button><br>
	<button  id=btdn>\/</button><td>
	<button  id=btright>&gt;</button>
	</table>
	<br>
	<button  id=bthit>HIT</button>
	<button  id=btsethome>G92 SET HOME</button>
	<button id=bthoming >G28 HOME</button>
	<br><br>
   <button id=btinit>Run Init</button>
	<button id=btpreview>Preview</button>
	<button id=btexecute>Execute</button>
   <br>
	<button id=btpause>PAUSE</button>
	<button id=btmotoroff>MOTOROFF</button>
	<br>
	<input size=10 id=edgcode><button id=btsend>Send</button>
	<div id=theads style="height:300px;display:block;overflow:scroll">
<script async src="//pagead2.googlesyndication.com/pagead/js/adsbygoogle.js"></script>
<!-- CNC-1 -->
<ins class="adsbygoogle"
     style="display:block"
     data-ad-client="ca-pub-9111193570003924"
     data-ad-slot="7668489344"
     data-ad-format="auto"></ins>
<script>
(adsbygoogle = window.adsbygoogle || []).push({});
</script>
	</div>
	</table>
	<script src="Potrace.js"></script>
	<script src="costy-x.js"></script>
	<script src="serial.js"></script>
<center>Butuh senter led ? kunjungi toko kami <a href="http://www.tokosenterled.com">Toko Senter kami</a><br>
<!-- Start of SimpleHitCounter Code -->
<div align="center"><a href="http://www.simplehitcounter.com" target="_blank"><img src="http://simplehitcounter.com/hit.php?uid=2360698&f=16777215&b=0" border="0" height="18" width="83" alt="web counter"></a><br><a href="http://www.simplehitcounter.com" target="_blank" style="text-decoration:none;"></a></div>
<!-- End of SimpleHitCounter Code -->
</center>

</body>

</html>
