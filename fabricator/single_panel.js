#!/usr/bin/env node

var paper = require('paper'), fs = require('fs');
var panelNumber = parseInt(process.argv[2] || '0');

function newDrawing() {
    var canvas = new paper.Canvas(5 * 12 * 72, 5 * 12 * 72, 'pdf');
    return require('./fabricator.pjs')(canvas);
}

with (newDrawing()) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panel = new Panel(growth, scale, panelNumber);

    MakeActualSizePage(panel.draw(), scale);
    view.update();
    fs.writeFileSync('panel-' + panelNumber + '-drawing.pdf', view.element.toBuffer());
}

with (newDrawing()) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panel = new Panel(growth, scale, panelNumber);

    MakeActualSizePage(panel.drawFront(), scale);
    view.update();
    fs.writeFileSync('panel-' + panelNumber + '-front.pdf', view.element.toBuffer());
}

with (newDrawing()) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panel = new Panel(growth, scale, panelNumber);

    MakeActualSizePage(panel.drawMiddle(), scale);
    view.update();
    fs.writeFileSync('panel-' + panelNumber + '-middle.pdf', view.element.toBuffer());
}

with (newDrawing()) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panel = new Panel(growth, scale, panelNumber);

    MakeActualSizePage(panel.drawBack(), scale);
    view.update();
    fs.writeFileSync('panel-' + panelNumber + '-back.pdf', view.element.toBuffer());
}
