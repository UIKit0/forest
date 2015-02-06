#!/usr/bin/env node

var paper = require('./lib/paper-node.js'),
    path = require('path'),
    fs = require('fs');

var outFile = path.resolve(__dirname, 'sticker.pdf')

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
        -2.214149,
        -2.192024,
        -2.173491,
        -2.165114,
        -2.163698,
        -2.171906,
        -2.160351,
        -2.138930,
        -2.132079,
        -2.169942,
        -2.130022,
        -2.122513,
        -2.104761,
        -2.070624,
        -2.057512,
        -2.001550,
        -1.968377,
        -1.938342,
        -1.908403,
        -1.833542,
        -1.689916,
        -1.569948,
        -1.496441,
        -1.370033,
        -1.396585,
        -1.276723,
        -1.084680,
        -0.934917,
        -0.742232,
        -0.727681,
        -0.481289,
        -0.435901,
        -0.239685,
        -0.067158,
        -0.054436,
        0.004210,
        0.112659,
        0.145134,
        0.179171,
        0.202146,
        0.132121,
        0.243570,
        0.199797,
        0.255575,
        0.250794,
        0.250339,
        0.268588,
        0.260460,
        0.300497,
        0.309578,
        0.308908,
        0.276954,
        0.299795,
        0.245581,
        0.311059,
        0.275466,
        0.326824,
        0.331250,
        0.343872,
        0.355479,
        0.364842,
        0.347236,
        0.356954,
        0.351240,
        0.362001,
        0.352438,
        0.383730,
        0.423033,
        0.386217,
        0.415806,
        0.465565,
        0.514711,
        0.537951,
        0.608070,
        0.655635,
        0.725748,
        0.769333,
        0.842263,
        0.899669,
        0.954499,
        0.998505,
        1.072836,
        1.080954,
        1.152972,
        1.071154,
        1.088223,
        1.090065,
        1.065149,
        1.082195,
        1.106680,
        1.114528,
        1.167594,
        1.118667,
        1.187315,
        1.176621,
        1.198528,
        1.212935,
        1.235997,
        1.291815,
        1.237187,
        1.222213,
        1.289809,
        1.264476,
        1.316183,
        1.393977,
        1.382300,
        1.489672,
        1.535203,
        1.604946,
        1.713625,
        1.805252,
        2.122813,
        1.831876,
        2.212900,
        2.468989,
        2.819137,
        3.075522,
        -3.095851,
        -2.807344,
        -2.624382,
        -2.538346,
        -2.431219,
        -2.392985,
        -2.335759,
        -2.334863,
        -2.293346,
        -2.288436,
        -2.235605,
        -2.237514,
        -2.193412,
        -2.180120,
        -2.167291,
        -2.135963,
        -2.139174,
        -2.118733,
        -2.121777,
        -2.110911,
        -2.087585,
        -2.087176,
        -2.105728
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
