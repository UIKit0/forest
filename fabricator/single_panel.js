#!/usr/bin/env node

var paper = require('paper'), fs = require('fs');
var panelNumber = parseInt(process.argv[2]);
var canvas = new paper.Canvas(5 * 12 * 72, 5 * 12 * 72, 'pdf');
var fab = require('./fabricator.pjs')(canvas);

with (fab) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();

    var panel = new Panel(growth, scale, panelNumber);
    var panelLayer = panel.draw()

    MakeActualSizePage(panelLayer, scale);
    view.update();
    fs.writeFileSync('panel-' + panelNumber + '.pdf', canvas.toBuffer());
}
