#!/usr/bin/env node

var paper = require('paper'),
    path = require('path'),
    fs = require('fs');

var inFile = path.resolve(__dirname, '../layout/growth.json')
var outFile = path.resolve(__dirname, 'panel.pdf')

var canvas = new paper.Canvas(612, 792, 'pdf');
var fab = require('./fabricator.pjs')(canvas);

with (fab) {
    fs.readFile(inFile, { encoding: 'utf8' }, function (err, data) {
        if (err) throw err;
        var growth = new Growth(JSON.parse(data));
        var scale = new Scale();

        var panel = new Panel(growth, scale, 0);
        var layer = panel.draw();

        var world = new Group();
        world.addChild(scale.drawGrid(layer.bounds));
        world.addChild(layer);

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
