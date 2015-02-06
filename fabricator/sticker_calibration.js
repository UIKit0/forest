#!/usr/bin/env node

var paper = require('./lib/paper-node.js'),
    path = require('path'),
    fs = require('fs');

var outFile = path.resolve(__dirname, 'sticker_calibration.pdf')

var canvas = new paper.Canvas(792, 612, 'pdf');
var fab = require('./fabricator.pjs')(canvas);

with (fab) {
    var scale = new Scale();
    var sticker = new Sticker(scale);

    MakeCalibrationPage(sticker.drawCalibrationDiagram(), scale);

    view.update();
    fs.writeFile(outFile, canvas.toBuffer(), function (err) {
        if (err)
            throw err;
        console.log('Saved!');
    });
}
