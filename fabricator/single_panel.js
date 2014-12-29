#!/usr/bin/env node

var paper = require('paper'),
    path = require('path'),
    fs = require('fs');

var panelNumber = parseInt(process.argv[2]);

var inFile = path.resolve(__dirname, '../layout/growth.json');
var outFile = path.resolve(__dirname, 'panel-' + panelNumber + '.pdf');

var canvas = new paper.Canvas(612, 792, 'pdf');
var fab = require('./fabricator.pjs')(canvas);

with (fab) {
    fs.readFile(inFile, { encoding: 'utf8' }, function (err, data) {
        if (err) throw err;
        var growth = new Growth(JSON.parse(data));
        var scale = new Scale();
        var world = new Group();

        var panel = new Panel(growth, scale, panelNumber);
        var panelLayer = panel.draw()

        world.addChild(panelLayer);
        world.addChild(scale.drawGrid(panelLayer.bounds));

        world.fitBounds(view.bounds.expand(-100));
        world.scale(1, -1);

        view.update();
        fs.writeFile(outFile, canvas.toBuffer(), function (err) {
            if (err)
                throw err;
            console.log('Saved!');
        });
    });
}
