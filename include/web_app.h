#pragma once

#include <Arduino.h>

static const char WEB_APP_JS[] PROGMEM = R"rawliteral(
const recordOutput = document.getElementById('record');
const recordStatus = document.getElementById('status');
const historyStatus = document.getElementById('history-status');
const exportButton = document.getElementById('export-csv');
const chart = document.getElementById('history-chart');
let gameScores = [];

exportButton.addEventListener('click', () => {
    const rows = ['game_number,score'];
    gameScores.forEach((score, index) => rows.push(`${index + 1},${score}`));

    const csv = new Blob(['\uFEFF', rows.join('\r\n')], { type: 'text/csv;charset=utf-8' });
    const url = URL.createObjectURL(csv);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'trex-results.csv';
    link.click();
    window.setTimeout(() => URL.revokeObjectURL(url), 1000);
});

function drawHistoryChart(scores) {
    const bounds = chart.getBoundingClientRect();
    const pixelRatio = window.devicePixelRatio || 1;
    chart.width = Math.max(1, Math.round(bounds.width * pixelRatio));
    chart.height = Math.max(1, Math.round(bounds.height * pixelRatio));

    const context = chart.getContext('2d');
    context.setTransform(pixelRatio, 0, 0, pixelRatio, 0, 0);

    const width = bounds.width;
    const height = bounds.height;
    const left = 42;
    const right = 12;
    const top = 18;
    const bottom = 30;
    const plotWidth = width - left - right;
    const plotHeight = height - top - bottom;
    const highestScore = Math.max(0, ...scores);
    const axisMaximum = Math.max(10, Math.ceil(highestScore / 10) * 10);

    context.clearRect(0, 0, width, height);
    context.font = '11px sans-serif';
    context.textBaseline = 'middle';
    context.textAlign = 'right';
    context.strokeStyle = '#8B8664';
    context.fillStyle = '#8B8664';
    context.lineWidth = 1;

    for (let tick = 0; tick <= 4; tick++) {
        const y = top + plotHeight * tick / 4;
        const value = Math.round(axisMaximum * (4 - tick) / 4);
        context.beginPath();
        context.moveTo(left, y);
        context.lineTo(width - right, y);
        context.stroke();
        context.fillText(String(value), left - 8, y);
    }

    context.textBaseline = 'alphabetic';
    context.textAlign = 'left';
    context.fillText('oldest', left, height - 5);
    context.textAlign = 'right';
    context.fillText('newest', width - right, height - 5);

    if (scores.length === 0) {
        context.textAlign = 'center';
        context.textBaseline = 'middle';
        context.fillStyle = '#FFFFFF';
        context.fillText('No games recorded', left + plotWidth / 2, top + plotHeight / 2);
        return;
    }

    const points = scores.map((score, index) => ({
        x: left + (scores.length === 1 ? plotWidth / 2 : plotWidth * index / (scores.length - 1)),
        y: top + plotHeight * (1 - score / axisMaximum)
    }));

    if (points.length > 1) {
        context.beginPath();
        context.moveTo(points[0].x, points[0].y);
        for (let index = 1; index < points.length; index++) {
            context.lineTo(points[index].x, points[index].y);
        }
        context.strokeStyle = '#BF9000';
        context.lineWidth = 2;
        context.shadowColor = '#BF9000';
        context.shadowBlur = 12;
        context.stroke();
        context.shadowBlur = 0;
    }

    for (const point of points) {
        context.beginPath();
        context.arc(point.x, point.y, 4, 0, Math.PI * 2);
        context.fillStyle = '#BF9000';
        context.shadowColor = '#BF9000';
        context.shadowBlur = 12;
        context.fill();
        context.shadowBlur = 0;

        context.beginPath();
        context.arc(point.x, point.y, 1.5, 0, Math.PI * 2);
        context.fillStyle = '#FFFFFF';
        context.fill();
    }
}

fetch('/api/record', { cache: 'no-store' })
    .then(response => {
        if (!response.ok) throw new Error('Request failed');
        return response.json();
    })
    .then(data => {
        recordOutput.textContent = String(data.record);
        recordStatus.textContent = '';
    })
    .catch(() => {
        recordOutput.textContent = '--';
        recordStatus.textContent = 'Here is no saved high score yet';
    });

fetch('/api/results', { cache: 'no-store' })
    .then(response => {
        if (!response.ok) throw new Error('Request failed');
        return response.json();
    })
    .then(data => {
        if (!Array.isArray(data.results)) throw new Error('Invalid results');
        gameScores = data.results.map(Number);
        if (gameScores.some(score => !Number.isFinite(score))) throw new Error('Invalid score');
        exportButton.disabled = gameScores.length === 0;
        historyStatus.textContent = `${gameScores.length} games stored`;
        drawHistoryChart(gameScores);
    })
    .catch(() => {
        historyStatus.textContent = 'Could not load game history';
        drawHistoryChart([]);
    });

window.addEventListener('resize', () => drawHistoryChart(gameScores));
)rawliteral";
