// https://gist.github.com/wteuber/6241786
function fmod(a,b) { return Number((a - (Math.floor(a / b) * b)).toPrecision(8)); }

// https://gamedev.stackexchange.com/questions/46463/how-can-i-find-an-optimum-set-of-colors-for-10-players
const COLORS = [
    Qt.hsva(fmod(0 * 0.618033988749895, 1.0), 0.5, 1.0),
    Qt.hsva(fmod(1 * 0.618033988749895, 1.0), 0.5, 1.0),
    Qt.hsva(fmod(2 * 0.618033988749895, 1.0), 0.5, 1.0),
    Qt.hsva(fmod(3 * 0.618033988749895, 1.0), 0.5, 1.0),
    Qt.hsva(fmod(4 * 0.618033988749895, 1.0), 0.5, 1.0),
    Qt.hsva(fmod(5 * 0.618033988749895, 1.0), 0.5, 1.0),
    Qt.hsva(fmod(6 * 0.618033988749895, 1.0), 0.5, 1.0),
    Qt.hsva(fmod(7 * 0.618033988749895, 1.0), 0.5, 1.0),
];

function nextColor(color) {
    return COLORS[(COLORS.indexOf(color) + 1) % COLORS.length];
}

function textColorFor(color) {
    if (color.value < 100) {
        return Qt.lighter(color, 3.0);
    }
    else {
        return Qt.darker(color, 3.0);
    }
}
