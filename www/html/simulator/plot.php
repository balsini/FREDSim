<script src="https://code.highcharts.com/highcharts.js"></script>
<script src="https://code.highcharts.com/modules/exporting.js"></script>

<script>
function plot(data) {

    //alert(data["debug"]);
    
    var myselectCSV = document.getElementById("containerCSV");
    myselectCSV.style.display = "inline";
    
    var csvData = '';
    
    {
        var row = 'x,';
        
        for (j = 0; j < data["data"].length; j++) {
            row += (data["data"][j]["name"]).toString();
            if (j < data["data"].length - 1) {
                row += ',';
            }
        }
    
        csvData += row + '\n';
    }
    for (i = 0; i < data["x"].length; i++) {
        var row = data["x"][i].toString() + ',';
        
        for (j = 0; j < data["data"].length; j++) {
            row += (data["data"][j]["data"][i]).toString();
            if (j < data["data"].length - 1) {
                row += ',';
            }
        }
    
        csvData += row + '\n';
    }
    
    myselectCSV.childNodes[0].innerHTML = csvData;//;
    
    var myselect = document.getElementById("container");
    myselect.style.display = "inline";
    
    $('#container').highcharts({
        title: {
            text: data["simulName"],
            x: -20 //center
        },
        xAxis: {
            title: {
                text: data["xName"],
            },
            categories: data["x"],
        },
        yAxis: {
            min: 0,
            max: 1,
            title: {
                text: 'Schedulable Tasksets (%)'
            },
            plotLines: [{
                value: 0,
                width: 1,
                color: '#808080'
            }]
        },
        tooltip: {
            valueSuffix: '%'
        },
        legend: {
            layout: 'vertical',
            align: 'right',
            verticalAlign: 'middle',
            borderWidth: 0
        },
        series: data["data"]/*
        [{
            name: 'TB_PREEMPTIVE_TODO',
            data: [7.0, 6.9, 9.5, 14.5, 18.2, 21.5, 25.2, 26.5, 23.3, 18.3, 13.9, 9.6]
        }, {
            name: 'TB_NONPREEMPTIVE_TODO',
            data: [-0.2, 0.8, 5.7, 11.3, 17.0, 22.0, 24.8, 24.1, 20.1, 14.1, 8.6, 2.5]
        }]*/
    });
};
</script>