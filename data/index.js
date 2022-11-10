var ctx = null;
var myChart;

function initPage(){
	//console.log( "ready!" );
	loadConfigFile();
	loadNetworkFile();
	
	$("#filenames").on("change", function() {
		//console.log(this.value);
		$("#download-file").attr("href", "file?f=" + this.value);
		loadCSVFile(this.value);
	});
	
	$("#btnParams").on("click", function() {
		var rg = $("#range-good").val();
		var ru = $("#range-unhealthy").val();
		var wi = $("#write-interval").val();
		var validate = true;
		$("#configuration input[type=number]").each(function(index, value) {
			$(this).attr("aria-invalid", "false");
			if($(this).is(":visible") && $.trim($(this).val()) == ''){
				$(this).attr("aria-invalid", "true");
				validate = false;
			}
		});
		if(validate){
			$.ajax({
				url: "/params",
				type: "GET",
				data: {
					rg: rg,
					ru: ru,
					wi: wi,
				},
			}).done(function( data ) {
				console.log(data);
				$(".lbl-good").html(rg);
				$(".lbl-unhealthy").html(ru);
				//update chart
				var fileName = $("#filenames").val();
				loadCSVFile(fileName);
			}).fail(function( xhr, status, errorThrown ) {
				console.log( "Error: " + errorThrown );
				console.log( "Status: " + status );
			}).always(function( xhr, status ) {
				//console.log( "The request is complete!" );
			});
		}
	});
	 
	$("#files table tbody").on("click", "a.del-file", function() {
		var f = $(this).attr("f");
		$(".file-date").attr("f", f).html(f.substring(7, 17));
		const modal = document.getElementById("modal-file"); 
		openModal(modal);
	});
	
	$("#mode").on("change", function() {
		//console.log(this.value);
		$("#network .grid").addClass("hidden");
		$("#network .desc").addClass("hidden");
		if(this.value == "2"){
			$(".dflt.grid").removeClass("hidden");
			$(".dflt.desc").removeClass("hidden");
		} else if(this.value == "0"){
			$(".ap.grid").removeClass("hidden");
			$(".ap.desc").removeClass("hidden");
		} else if(this.value == "1"){
			$(".st.grid").removeClass("hidden");
			$(".st.desc").removeClass("hidden");
		}	
	});
	
	$("#btnNetwork").on("click", function() {
		var mod = $("#mode").val();
		var aps = $("#ap_ssid").val();
		var app = $("#ap_pasw").val();
		var sts = $("#st_ssid").val();
		var stp = $("#st_pasw").val();
		var validate = true;
		$("#network input[type=text]").each(function(index, value) {
			$(this).attr("aria-invalid", "false");
			if($(this).is(":visible") && $.trim($(this).val()) == ''){
				$(this).attr("aria-invalid", "true");
				validate = false;
			}
		});
		if(validate){
			//Modal restart page
			$("#modal-network .modal-body").html($("#network .desc:not(.hidden)").html());
			const modal = document.getElementById("modal-network"); 
			openModal(modal);
			//
			$.ajax({
				url: "/network",
				type: "GET",
				data: {
					mod: mod,
					aps: aps,
					app: app,
					sts: sts,
					stp: stp,
				},
			}).done(function( data ) {
				console.log(data);
				console.log("network update");
				
			}).fail(function( xhr, status, errorThrown ) {
				console.log( "Error: " + errorThrown );
				console.log( "Status: " + status );
			}).always(function( xhr, status ) {
				//console.log( "The request is complete!" );
			});
		}
	});
	
	$("#btnCalibrate").on("click", function() {
		var validate = true;
		$("#calibrate input[type=number]").each(function(index, value) {
			$(this).attr("aria-invalid", "false");
			if($(this).is(":visible") && $.trim($(this).val()) == ''){
				$(this).attr("aria-invalid", "true");
				validate = false;
			}
		});
		if(validate){
			var crf = $("#cal_ref").val();
			//Modal restart page
			//$("#modal-network .modal-body").html($("#network .desc:not(.hidden)").html());
			//const modal = document.getElementById("modal-network"); 
			//openModal(modal);
			//
			$.ajax({
				url: "/calibrate",
				type: "GET",
				data: {
					crf: crf,
				},
			}).done(function( data ) {
				console.log(data);
				console.log("calibrate init");
			}).fail(function( xhr, status, errorThrown ) {
				console.log( "Error: " + errorThrown );
				console.log( "Status: " + status );
			}).always(function( xhr, status ) {
				//console.log( "The request is complete!" );
			});
		}
	});
}

function loadConfigFile() {
	var fileNameDD = $("#filenames");
	fileNameDD.empty();
	$.ajax({
		url: "/paramsfile",
		type: "GET",
	}).done(function( data ) {
		//
		console.log(data);
		var params = data.split("\n");
		var rg = params[0].split(":")[1];
		var ru = params[1].split(":")[1];
		var wi = params[2].split(":")[1];
		//	
		//console.log("rg: " + rg);
		//console.log("ru: " + ru);
		//console.log("wi: " + wi);
		//
		$("#range-good").val(rg);
		$("#range-unhealthy").val(ru);
		$("#write-interval").val(wi);
		$(".lbl-good").html(rg);
		$(".lbl-unhealthy").html(ru);
			
		loadCSVFiles(); 
		//console.log("end");
	}).fail(function( xhr, status, errorThrown ) {
		//alert( "Sorry, there was a problem!" );
		console.log( "Error: " + errorThrown );
		console.log( "Status: " + status );
		//console.dir( xhr );
	}).always(function( xhr, status ) {
		//console.log( "The request is complete!" );
	});
}

function loadNetworkFile() {
	$.ajax({
		url: "/networkfile",
		type: "GET",
	}).done(function( data ) {
		//
		console.log(data);
		var params = data.split("\n");
		var mod = params[0].split(":")[1];
		var aps = params[1].split(":")[1];
		var app = params[2].split(":")[1];
		var sts = params[3].split(":")[1];
		var stp = params[4].split(":")[1];
		//
		//console.log("mod: " + mod);
		//console.log("aps: " + aps);
		//console.log("app: " + app);
		//console.log("sts: " + sts);
		//console.log("stp: " + stp);
		//
		$("#ap_ssid").val(aps);
		$("#ap_pasw").val(app);
		$("#st_ssid").val(sts);
		$("#st_pasw").val(stp);
		
		$("#mode").val(mod).trigger('change');
		//console.log("end");
	}).fail(function( xhr, status, errorThrown ) {
		//alert( "Sorry, there was a problem!" );
		console.log( "Error: " + errorThrown );
		console.log( "Status: " + status );
		//console.dir( xhr );
	}).always(function( xhr, status ) {
		//console.log( "The request is complete!" );
	});
}

function loadCSVFiles() {
	var fileNameDD = $("#filenames");
	fileNameDD.empty();
	$.ajax({
		url: "/files",
		type: "GET",
	}).done(function( data ) {
		var filesTb = $("#files table tbody");
		filesTb.empty();
		const fileNames = data.split(",");
		var filesCSVHTML = [];
		for(i=0; i<fileNames.length; i++){
			if(fileNames[i]!=""){
				fileNameDD.append($("<option />").val(fileNames[i]).text(fileNames[i].substring(7, 17)));
				//filesCSVHTML.push();
				filesCSVHTML.push("<tr>");
				filesCSVHTML.push("<th scope='row'>" + fileNames[i].substring(7, 17) + "</th>");
				filesCSVHTML.push("<td><a href='file?f=" + fileNames[i] + "'>Download</a></td>");
				filesCSVHTML.push("<td><a class='del-file' href='#' data-target='modal-file' f='" + fileNames[i] + "'>Delete</a></td>");
				filesCSVHTML.push("</tr>");
			}
		}
		filesTb.append(filesCSVHTML.join(""));
		//trigger
		fileNameDD.val($("option:last", fileNameDD).val()).trigger('change');
		//console.log("end");
	}).fail(function( xhr, status, errorThrown ) {
		//alert( "Sorry, there was a problem!" );
		console.log( "Error: " + errorThrown );
		console.log( "Status: " + status );
		//console.dir( xhr );
	}).always(function( xhr, status ) {
		//console.log( "The request is complete!" );
	});
}

function loadCSVFile(fileName){
	$.ajax({
		url: "/file",
		type: "GET",
		data: {
			f: fileName
		},
	}).done(function( data ) {
		//console.log(data);
		doChart(data); 
	}).fail(function( xhr, status, errorThrown ) {
		console.log( "Error: " + errorThrown );
		console.log( "Status: " + status );
	}).always(function( xhr, status ) {
		//console.log( "The request is complete!" );
	});
}
 
function doChart(contentcsv){
	var DateTime = luxon.DateTime;
	var dataArray = parseCSV(contentcsv);
	//console.log(dataArray.length);
	var timeLabels = dataArray.map(function (d) {
		return DateTime.fromISO(d.time).toFormat("TT");
	});
	var co2Data = dataArray.map(function (d) {
		return d.co2;
	});
	var colorData = dataArray.map(function (d) {
		return d.color;
	});
	//$(".lastvalue-holder span").remove();
	$(".lastvalue-holder input").css("width","50%");
	var co2 = co2Data[co2Data.length - 1];
	var timeLbl = timeLabels[timeLabels.length - 1];
	$(".lastvalue-holder input").val( co2 + " - " + timeLbl);
	//$(".lastvalue-holder").append("<span>&nbsp;</span>");
	$(".lastvalue-holder span").css({
			'width' : '20%',
			'display':'inline-table',
			'border-radius':'0.25rem',
			'background-color': colorData[colorData.length - 1]
		});
	if(ctx == null){ 
		ctx = document.getElementById('myChart').getContext('2d'); // 2d context
		myChart = new Chart(ctx, {
			type:"bar",
			data: {
				labels: timeLabels,
				datasets: [
				  {
					label: 'C02',
					display:false,
					data: co2Data,
					backgroundColor: colorData,
				  }
				]
			},
			options: {
				animation: {
					duration: 0
				},
				plugins: {
				  title: {
					text: 'CO2',
					display: false
				  }
				},
				scales: {
					y: {
						beginAtZero: false,
					}
				}
			}
		});
	} else {
		myChart.data = {
			labels: timeLabels,
			datasets: [
				{
					label: 'C02',
					display:false,
					data: co2Data,
					backgroundColor: colorData,
				}
			]
		};
		myChart.update();	
	}
}

function parseCSV(contentcsv) {
	var rg = parseFloat($("#range-good").val());
	var ru = parseFloat($("#range-unhealthy").val());
    var array = [];
    var lines = contentcsv.split("\n");
	//skip column names
    for (var i = 1; i < lines.length; i++) {
        var data = lines[i].split(",", 2);
		var co2 = parseFloat(data[1]);
		var c = "";
		if(co2<rg) {
			//c = 'rgba(0, 189, 20, 0.2)';
			c = 'rgba(56, 142, 60, 0.8)';
		} else if(co2<ru) {
			//c = 'rgba(255, 206, 86, 0.2)';
			c = 'rgba(231, 176, 11, 0.8)';
		} else {
			//c = 'rgba(255, 99, 132, 1)';
			c = 'rgba(198, 40, 40, 0.8)';
		}
		var ds = {
			time: data[0],//DateTime.fromISO(data[0]),
			co2: co2,
			color: c
		};
        array.push(ds);
    }
    return array;
}
   
function deleteCSVFile(fileName){
	$.ajax({
		url: "/reset",
		type: "GET",
		data: {
			f: fileName
		},
	}).done(function( data ) {
		//console.log(data);
		const modal = document.getElementById("modal-file"); 
		closeModal(modal);
		//reset files
		setInterval(function ( ) {
			loadCSVFiles();
		}, 3000 );
	}).fail(function( xhr, status, errorThrown ) {
		console.log( "Error: " + errorThrown );
		console.log( "Status: " + status );
	}).always(function( xhr, status ) {
		//console.log( "The request is complete!" );
	});
}

function confirmModalFile(event){
    var f =  $(".file-date").attr("f");
	deleteCSVFile(f);
}

function confirmModalCalibrate(event){
	var validate = true;
	$("#modal-calibrate input[type=number]").each(function(index, value) {
		$(this).attr("aria-invalid", "false");
		if($(this).is(":visible") && $.trim($(this).val()) == ''){
			$(this).attr("aria-invalid", "true");
			validate = false;
		}
	});
	if(validate){
		var crf = $("#cal_ref").val();
		//Modal restart page
		//$("#modal-network .modal-body").html($("#network .desc:not(.hidden)").html());
		//const modal = document.getElementById("modal-network"); 
		//openModal(modal);
		//
		$.ajax({
			url: "/calibrate",
			type: "GET",
			data: {
				crf: crf,
			},
		}).done(function( data ) {
			console.log(data);
			console.log("calibrate init");
		}).fail(function( xhr, status, errorThrown ) {
			console.log( "Error: " + errorThrown );
			console.log( "Status: " + status );
		}).always(function( xhr, status ) {
			//console.log( "The request is complete!" );
		});
	}
}