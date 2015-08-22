String.prototype.capitalizeFirstLetter = function () {
    return this.charAt(0).toUpperCase() + this.slice(1);
}

jQuery(function ($) {
    var updateInterval = window.setInterval(
        function () {
            update();
        },
        20000
    );

    var timestampUpdateInterval = window.setInterval(updateTimestamps, 1000);

    update();

    var lastUpdate = null;
    var lastChange = null;
    var buzzerStatus = '???';

    function update() {
        var response = $.get(
            'get-status-json'
        )

        response.done(function (data) {
            lastUpdate = moment(data.lastUpdate.date);
            if ($('body').attr('class') != data.class) {
                $('#box').fadeOut('2000', function () {
                    $('#activity-indicator').hide();
                    $('#overlay').fadeOut();
                    $('body').attr('class', data.class);
                    $('#status').text(data.text);
                    lastChange = moment(data.lastChange.date);
                    $('#buzzerStatus').text(data.buzzerStatus);

                    // Make the text fit nicely
                    if ($(window).width() > 480) {
                        $('#status').css('font-size', parseInt(5 + 20 / data.text.length) + 'rem');
                    }
                    $(this).fadeIn();
                })
            }


        })
            .fail(function (data) {
                $('body').attr('class', '???');
                $('#status').text("???");
                $('#buzzerStatus').text('???');
            })
    }

    function updateTimestamps() {
        if (null !== lastUpdate) {
            $('#lastChange').attr('title', lastChange.format('D.M.YYYY HH:mm:ss'));
            $('#lastChange').text(lastChange.fromNow().capitalizeFirstLetter());
        }
        if (null !== lastChange) {
            $('#lastUpdate').attr('title', lastUpdate.format('D.M.YYYY HH:mm:ss'));
            $('#lastUpdate').text(lastUpdate.fromNow().capitalizeFirstLetter());
        }
    }

    $('#simulate span').click(function(e) {
        $('#spinner').show();
        if ($(this).attr('id') == 'ok') {
            var waterlevel = 0
        } else {
            var waterlevel = 3
        }
        $.ajax({
            url: 'post-status?waterlevel=' + waterlevel + '&muted=0',
            success: function() {
                $('#spinner').hide();
            }
        })
    })
})