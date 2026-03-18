let ui = {
    animated: false,
    headers: { 'Content-type': 'application/json' },
    element: function (id) {
        return document.getElementById(id);
    },
    timestamp: function (id) {
        let d = new Date();
        let t = document.querySelector('#' + id).value.padStart(6, 0) || d.getHours().toString().padStart(2, 0) + d.getMinutes().toString().padStart(2, 0) + d.getSeconds().toString().padStart(2, 0);
        d.setHours(t.substr(0, 2), t.substr(2, 2), t.substr(4, 2), 0);
        return Math.round(d.getTime() / 1000);
    },
    military: function (d) {
        return d.getHours().toString().padStart(2, '0') +
            d.getMinutes().toString().padStart(2, '0') +
            d.getSeconds().toString().padStart(2, '0');
    },
    epoch: function (d) {
        return Math.floor(d.getTime() / 1000);
    },
    date: function (t) {
        let date = new Date();
        if (t) date.setTime(t * 1000);
        return date;
    },
    state: 'IDLE',
    last: 0,
};


function updateActuator(d) {
    ui.element('az_deg').innerText = Math.round(d.az_deg);
    ui.element('az_pos').innerText = d.az_pos;
    ui.element('el_deg').innerText = Math.round(d.el_deg);
    ui.element('el_pos').innerText = d.el_pos;
}

function updateTracker(d) {
    ui.state = d.state;
    ui.element('tracker').classList.remove('idle', 'execute', 'standby', 'track', 'park');
    ui.element('tracker').classList.add(d.state?.toLowerCase());
    ui.element('mel_el').value = d.mel_el;
    ui.element('aos_time').value = d.aos_time ? ui.military(ui.date(d.aos_time)) : 0;
    ui.element('aos_az').value = d.aos_az;
    ui.element('aos_el').value = d.aos_el;
    ui.element('los_time').value = d.los_time ? ui.military(ui.date(d.los_time)) : 0;
    ui.element('los_az').value = d.los_az;
    ui.element('los_el').value = d.los_el;
    ui.element('state').innerText = d.state;
    ui.element('cd').innerText = d.cd;
}

function update(t) {
    // update the time
    ui.element('cur_time').innerText = ui.military(ui.date());

    // animate our patient
    if (!ui.animated) {
        fetch('/time', {
            method: 'POST',
            headers: ui.headers,
            body: JSON.stringify({
                value: Math.floor((new Date()).getTime() / 1000),
            }),
        }).then(r => {
            if (r.ok) {
                ui.animated = true;
                return;
            }

            throw Error('set time failed');
        }).catch(e => { });
    }

    if ('IDLE' !== ui.state) {
        fetch('/actuator/state', {
            method: 'GET',
            headers: ui.headers,
        }).then(r => {
            if (r.ok) return r.json();
            throw Error('fetch ' + url + ' failed');
        }).then(d => {
            updateActuator(d);
        }).catch(e => { });

        fetch('/tracker/state', {
            method: 'GET',
            headers: ui.headers,
        }).then(r => {
            if (r.ok) return r.json();
            throw Error('fetch ' + url + ' failed');
        }).then(d => {
            updateTracker(d);
        }).catch(e => { });
    }
}

function loop(t) {
    let dt = t - ui.last;

    if (0 === ui.last || dt >= 1000) {
        update();
        ui.last = t;
    }

    requestAnimationFrame(loop);
}


document.querySelectorAll('#actuator button[name="move"]').forEach(el => {
    el.addEventListener('click', () => {
        fetch('/actuator/move', {
            method: 'POST',
            headers: ui.headers,
            body: JSON.stringify({
                'value': el.value,
                'axis': el.dataset.axis,
                'mode': el.dataset.mode,
            }),
        }).then(r => {
            if (r.ok) return r.json();
            throw new Error('actuator POST failed');
        }).then(d => {
            updateActuator(d);
        }).catch(e => {
        });
    });
});

document.querySelectorAll('#actuator button[name="calibrate"]').forEach(el => {
    el.addEventListener('click', () => {
        fetch('/actuator/calibrate', {
            method: 'POST',
            headers: ui.headers,
            body: JSON.stringify({
                'axis': el.dataset.axis,
            }),
        }).then(r => {
            if (r.ok) return r.json();
            throw new Error('actuator calibration failed');
        }).then(d => {
            updateActuator(d);
        }).catch(e => {
        });
    });
});

document.querySelectorAll('#actuator button[name="reset"]').forEach(el => {
    el.addEventListener('click', () => {
        fetch('/actuator/reset', {
            method: 'POST',
            headers: ui.headers,
            body: JSON.stringify({
                'axis': el.dataset.axis,
            }),
        }).then(r => {
            if (r.ok) return r.json();
            throw new Error('actuator reset failed');
        }).then(d => {
            updateActuator(d);
        }).catch(e => {
        });
    });
});

document.querySelectorAll('#tracker button[name="execute"]').forEach(el => {
    el.addEventListener('click', () => {
        fetch('/tracker/execute', {
            method: 'POST',
            headers: ui.headers,
            body: '{}',
        }).then(r => {
            if (r.ok) return r.json();
            throw new Error('tracker execute failed');
        }).then(d => {
            updateTracker(d);
        }).catch(e => {
        });
    });
});

document.querySelectorAll('#tracker button[name="set"]').forEach(el => {
    el.addEventListener('click', () => {
        fetch('/tracker/set', {
            method: 'POST',
            headers: ui.headers,
            body: JSON.stringify({
                'aos_time': ui.timestamp('aos_time'),
                'los_time': ui.timestamp('los_time'),
                'aos_az': ui.element('aos_az').value || 0,
                'aos_el': ui.element('aos_el').value || 0,
                'los_az': ui.element('los_az').value || 0,
                'los_el': ui.element('los_el').value || 0,
                'mel_el': ui.element('mel_el').value || 0,
            }),
        }).then(r => {
            if (r.ok) return r.json();
            throw new Error('tracker set failed');
        }).then(d => {
            updateTracker(d);
        }).catch(e => {
        });
    });
});

document.querySelectorAll('#tracker button[name="reset"]').forEach(el => {
    el.addEventListener('click', () => {
        fetch('/tracker/reset', {
            method: 'DELETE',
            headers: ui.headers,
        }).then(r => {
            if (r.ok) return r.json();
            throw new Error('tracker reset failed');
        }).then(d => {
            updateTracker(d);
        }).catch(e => {
        });
    });
});

requestAnimationFrame(loop);
