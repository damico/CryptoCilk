<?php


ini_set("display_errors", 1);
ini_set("track_errors", 1);
ini_set("html_errors", 1);
error_reporting(E_ALL);


include('adodb/adodb5/adodb.inc.php');

$server = "127.0.0.1";
$user = "jinwork";
$pwd = "ignition";
$db = "CryptoCilk";

?>

<!DOCTYPE html>
<html>
<head>
  <meta http-equiv="content-type" content="text/html; charset=UTF-8">
  
<title>CryptoCilk Report</title>

<script type="text/javascript" src="jquery/jquery-1.8.0.min.js"></script>
<script language="javascript" type="text/javascript" src="dist/jquery.jqplot.js"></script>
<script src="dist/plugins/jqplot.pieRenderer.min.js"></script>
<script type="text/javascript" src="dist/plugins/jqplot.barRenderer.min.js"></script>
<script type="text/javascript" src="dist/plugins/jqplot.categoryAxisRenderer.min.js"></script>
<script type="text/javascript" src="dist/plugins/jqplot.pointLabels.min.js"></script>
<script src="dist/plugins/jqplot.donutRenderer.min.js"></script>


<link rel="stylesheet" type="text/css" href="dist/jquery.jqplot.min.css" />
  
  <style type='text/css'>
    #chart{
    width: 100%;
    height: 200;

    }
</style>

<script type="text/javascript">
  setTimeout(function(){
    location = ''
  },120000)
</script>

<?php
$data1 = "";
$data2 = "";


$DB = NewADOConnection('postgres');
$DB->Connect($server, $user, $pwd, $db);
$query = "SELECT proc_log_time,proc_log_timestamp FROM crypto.proc_log_t where proc_log_paral = 0 order by proc_log_timestamp DESC";
$result = $DB->Execute($query) or die("Error in query: $query. " . $DB->ErrorMsg());
$i = 0;
while (!$result->EOF)
{


 $data1 =  "".$result->fields[0]."" . "," . $data1;

   $result->MoveNext();
   $i++;
}
$DB->Close();
$data1 = "[".substr($data1, 0,  strlen($data1)-1)."]";


$DB = NewADOConnection('postgres');
$DB->Connect($server, $user, $pwd, $db);
$query = "SELECT proc_log_time,proc_log_timestamp FROM crypto.proc_log_t where proc_log_paral = -1 order by proc_log_timestamp DESC";
$result = $DB->Execute($query) or die("Error in query: $query. " . $DB->ErrorMsg());
$i = 0;
while (!$result->EOF)
{
   $data_timestamp = ($result->fields[0] ); 
 $data2 =  "".$result->fields[0]."" . "," . $data2;

   $result->MoveNext();
   $i++;
}
$DB->Close();
$data2 = "[".substr($data2, 0,  strlen($data2)-1)."]";



$DB = NewADOConnection('postgres');
$DB->Connect($server, $user, $pwd, $db);
$query = "SELECT AVG(proc_log_time) FROM crypto.proc_log_t where proc_log_paral = -1 ";
$result = $DB->Execute($query) or die("Error in query: $query. " . $DB->ErrorMsg());
while (!$result->EOF)
{
   $for = ($result->fields[0] );
   $result->MoveNext();
}
$DB->Close();


$DB = NewADOConnection('postgres');
$DB->Connect($server, $user, $pwd, $db);
$query = "SELECT AVG(proc_log_time) FROM crypto.proc_log_t where proc_log_paral = 0";
$result = $DB->Execute($query) or die("Error in query: $query. " . $DB->ErrorMsg());
while (!$result->EOF)
{
   $cilk_for = ($result->fields[0] );
   $result->MoveNext();
}
$DB->Close();


?>  


<script class="code" type="text/javascript">  

$(document).ready(function() {
var lines = <?php echo($data1); ?>;
var lines1 = <?php echo($data2); ?>;
   $.jqplot('chart', [lines,lines1],{

      seriesDefaults: { 
        showMarker:false,
        pointLabels: { show:true }
      }


});
});

$(document).ready(function() {


var slice_1 = ['cilk_for', <?php echo $cilk_for; ?>];
  var slice_2 = ['for', <?php echo $for; ?>];
  var series = [slice_1, slice_2];
  var data = [series];
 
var options = {
  title: 'Time AVG',
  seriesDefaults: {
    renderer: jQuery.jqplot.DonutRenderer,
    rendererOptions: {
      showDataLabels: true,
      dataLabels: 'value',
      fill: false,
      sliceMargin: 5,
      startAngle: 45
    }
  },
  legend: { show:true, location: 'e' }
};
   
  $.jqplot('chart1', data, options);


});
</script>




	






</head>
<body>
<h2>CryptoCilk</h2><a href="http://github.com/damico/CryptoCilk">http://github.com/damico/CryptoCilk</a><hr><br>
<div id="chart"></div><br><br>
<div id="chart1" style="margin-top:20px; margin-left:20px; width:600px; height:300px;"></div><br><br>
<br>
<hr>
<a href="http://github.com/damico/CryptoCilk">http://github.com/damico/CryptoCilk</a>
</body>


</html>

