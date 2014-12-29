#!/usr/bin/env node

var paper = require('paper'),
    path = require('path'),
    fs = require('fs');

var inFile = path.resolve(__dirname, '../layout/growth.json')
var outFile = path.resolve(__dirname, 'growth.pdf')

var canvas = new paper.Canvas(792, 612, 'pdf');
var fab = require('./fabricator.pjs')(canvas);

with (fab) {
    fs.readFile(inFile, { encoding: 'utf8' }, function (err, data) {
        if (err) throw err;
        var growth = new Growth(JSON.parse(data));
        var scale = new Scale();
        var world = new Group();

        MakeDiagramPage(growth.draw(), scale);

        view.update();
        fs.writeFile(outFile, canvas.toBuffer(), function (err) {
            if (err)
                throw err;
            console.log('Saved!');
        });
    });
}
