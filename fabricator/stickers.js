#!/usr/bin/env node

var paper = require('paper'),
    path = require('path'),
    fs = require('fs');

var outFile = path.resolve(__dirname, 'stickers.pdf')

var canvas = new paper.Canvas(792, 612, 'pdf');
var fab = require('./fabricator.pjs')(canvas);

with (fab) {
    var scale = new Scale();
    var sticker = new Sticker(scale);
    var group = new Group();

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
