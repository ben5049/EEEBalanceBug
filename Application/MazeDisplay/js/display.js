/* Integrated into front end */

function drawLine(ctx, begin, end, stroke = 'black', width = 1) {
    if (stroke) {
        ctx.strokeStyle = stroke;
    }

    if (width) {
        ctx.lineWidth = width;
    }

    ctx.beginPath();
    ctx.moveTo(...begin);
    ctx.lineTo(...end);
    ctx.stroke();
}
// draws small line in correct direction with correct distances using TOF sensors
function draw(position, orientation, tofleft, tofright) {
    const canvas = document.querySelector('#canvas');

    if (canvas.getContext) {
        const ctx = canvas.getContext('2d');
        let l = 5;
        let theta = orientation*Math.PI/180
    
        drawLine(ctx, [position[0] + tofleft * Math.cos(theta), position[1] + tofleft * Math.sin(theta)], [position[0] + tofleft * Math.cos(theta) + l * Math.sin(theta), position[1] + tofleft * Math.sin(theta) - l * Math.cos(theta)]);
        drawLine(ctx, [position[0] - tofright * Math.cos(theta), position[1] - tofright * Math.sin(theta)], [position[0] - tofright * Math.cos(theta) + l * Math.sin(theta), position[1] - tofright * Math.sin(theta) - l * Math.cos(theta)]);
    }
}

