#!/usr/bin/env node

var paper = require('./lib/paper-node.js'),
    path = require('path'),
    fs = require('fs');

var outFile = path.resolve(__dirname, 'stickers.pdf')

var canvas = new paper.Canvas(792, 612, 'pdf');
var fab = require('./fabricator.pjs')(canvas);

with (fab) {
    var scale = new Scale();
    var sticker = new Sticker(scale);
    var group = new Group();

    /*
     * To linearize the proportional relationship between detected RGB vector angle
     * and physical angle, we start with a generic sine wave pattern in RGB space
     * and apply a calibration step in order to stretch the original pattern into
     * a more circular shape.
     *
     * To perform this test, print the sticker_calibration.js output, expose the
     * sensor evenly to all test squares, then log the angles recorded for each
     * numbered square in order.
     *
     * Paste the results into the table below.
     */

    sticker.calibrate([
        0.546972,
        0.615971,
        0.687055,
        0.790849,
        0.844397,
        0.957747,
        1.092013,
        1.123356,
        1.138776,
        1.236858,
        1.469568,
        1.647375,
        2.389403,
        -2.924253,
        -2.476896,
        -2.290024,
        -2.216104,
        -2.156897,
        -2.146922,
        -2.066358,
        -1.982480,
        -1.896829,
        -1.421427,
        -0.811407,
        -0.193346,
        0.188272,
        0.303156,
        0.401853,
        0.455132,
        0.468610,
        0.484770,
        0.534549,
    ]);

    group.addChild(sticker.drawAlignmentMarks());
    group.addChild(sticker.draw());

    MakeActualSizePage(group, scale);

    view.update();
    fs.writeFile(outFile, canvas.toBuffer(), function (err) {
        if (err)
            throw err;
        console.log('Saved!');
    });
}
