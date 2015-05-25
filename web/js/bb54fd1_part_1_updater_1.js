jQuery(function ($) {
    var updateInterval = window.setInterval(
        function () {
            update();
        },
        20000
    );

    var timestampUpdateInterval = window.setInterval( updateTimestamps, 1000);

    window.setTimeout(update, 5000);

    var lastUpdate = null;
    var lastChange = null;
    var buzzerStatus = '???';

    function update() {
        var response = $.get(
            'get-status-json'
        )

        response.done(function (data) {
            $('body').attr('class', data.class);
            $('#status').text(data.text);
            lastUpdate = moment(data.lastUpdate.date);
            lastChange = moment(data.lastChange.date);
            $('#buzzerStatus').text(data.buzzerStatus);

        })
            .fail(function (data) {
                $('body').attr('class', '???');
                $('#status').text("???");
                $('#buzzerStatus').text('???');
            })
    }

    function updateTimestamps() {
        if (null !== lastUpdate) {
            $('#lastChange').attr('title', lastChange.format('D.M.YYYY HH:mm:ss') );
            $('#lastChange').text(lastChange.fromNow());
        }
        if (null !== lastChange) {
            $('#lastUpdate').attr('title', lastUpdate.format('D.M.YYYY HH:mm:ss') );
            $('#lastUpdate').text(lastUpdate.fromNow());
        }
    }
})