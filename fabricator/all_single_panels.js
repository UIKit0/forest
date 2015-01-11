#!/usr/bin/env node

var paper = require('./lib/paper-node.js'), fs = require('fs');
var panelNumber = parseInt(process.argv[2] || '0');

function newDrawing(inchWidth, inchHeight) {
    var canvas = new paper.Canvas(inchWidth * 72, inchHeight * 72, 'pdf');
    return require('./fabricator.pjs')(canvas);
}

with (newDrawing(8.5, 11)) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panel = new Panel(growth, scale, panelNumber);

    MakeDiagramPage(panel.draw(), scale);
    view.update();
    fs.writeFileSync('panel-' + panelNumber + '-drawing.pdf', view.element.toBuffer());
}

with (newDrawing(5*12, 5*12)) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panel = new Panel(growth, scale, panelNumber);

    MakeActualSizePage(panel.drawFront(), scale);
    view.update();
    fs.writeFileSync('panel-' + panelNumber + '-front.pdf', view.element.toBuffer());
}

with (newDrawing(5*12, 5*12)) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panel = new Panel(growth, scale, panelNumber);

    MakeActualSizePage(panel.drawMiddle(), scale);
    view.update();
    fs.writeFileSync('panel-' + panelNumber + '-middle.pdf', view.element.toBuffer());
}

with (newDrawing(5*12, 5*12)) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panel = new Panel(growth, scale, panelNumber);

    MakeActualSizePage(panel.drawBack(), scale);
    view.update();
    fs.writeFileSync('panel-' + panelNumber + '-back.pdf', view.element.toBuffer());
}
