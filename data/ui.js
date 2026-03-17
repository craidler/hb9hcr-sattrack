function timestamp(id) {
    let d = new Date();
    let t = document.querySelector('#' + id).value.padStart(4, 0) || d.getHours().toString().padStart(2, 0) + d.getMinutes().toString().padStart(2, 0);
    d.setHours(t.substr(0, 2), t.substr(2, 2), 0, 0);
    return d.getTime();
}

function set(id, value) {
    document.querySelector('#' + id).innerText = value;
}

function value(id, value) {
    document.querySelector('#' + id).value = value;
}

function setActuator(data) {
    const date = new Date(data.time * 1000);
    set('az_deg', data.az_deg.toFixed(2));
    set('el_deg', data.el_deg.toFixed(2));
    set('az_pos', data.az_pos.toFixed(2));
    set('el_pos', data.el_pos.toFixed(2));
    set('time', date.getHours().toString().padStart(2, '0') + date.getMinutes().toString().padStart(2, '0'));
}

function setState(s) {
    if ('IDLE' === s) {
        document.querySelector('#tracker').classList.remove('running');
        return s;
    }

    document.querySelector('#tracker').classList.add('running');
    return s;
}

function setTracker(data) {
    const date = new Date(data.time * 1000);
    const aos = new Date(data.aos * 1000);
    const los = new Date(data.los * 1000);
    value('aos_time', aos.getHours().toString().padStart(2, '0') + aos.getMinutes().toString().padStart(2, '0'));
    value('aos_az', data.aos_az);
    value('aos_el', data.aos_el);
    value('los_time', los.getHours().toString().padStart(2, '0') + los.getMinutes().toString().padStart(2, '0'));
    value('los_az', data.los_az);
    value('los_el', data.los_el);
    value('mel_el', data.mel_el);
    set('state', setState(data.state));
    set('time', date.getHours().toString().padStart(2, '0') + date.getMinutes().toString().padStart(2, '0'));
    set('cd', data.cd);
}

document.querySelectorAll('button[name="mve"]').forEach(el => {
    el.addEventListener('click', () => {
        fetch('/actuator', {
            method: 'POST',
            headers: { 'Content-type': 'application/json' },
            body: JSON.stringify({
                'axis': el.dataset.axis,
                'mode': el.dataset.mode,
                'value': el.value,
            }),
        }).then(r => {
            if (!r.ok) throw new Error('actuator POST failed');
            return r.json();
        }).then(d => setActuator(d)).catch(e => {
            set('log', e.message);
        });
    });
});

document.querySelectorAll('button[name="rst"]').forEach(el => {
    el.addEventListener('click', () => {
        fetch('/actuator/' + el.dataset.axis, {
            method: 'DELETE',
            headers: { 'Content-type': 'application/json' },
        }).then(r => {
            if (!r.ok) throw new Error('actuator DELETE failed');
            return r.json();
        }).then(d => setActuator(d)).catch(e => {
            set('log', e.message);
        });
    });
});

document.querySelector('button[name="exe"]').addEventListener('click', () => {
    fetch('/tracker', {
        method: 'POST',
        headers: { 'Content-type': 'application/json' },
    }).then(r => {
        if (!r.ok) throw new Error('tracker POST failed');
        return r.json();
    }).then(d => setTracker(d)).catch(e => {
        set('log', e.message);
    });
});

document.querySelector('button[name="clr"]').addEventListener('click', () => {
    fetch('/tracker', {
        method: 'DELETE',
        headers: { 'Content-type': 'application/json' },
    }).then(r => {
        if (!r.ok) throw new Error('tracker DELETE failed');
        return r.json();
    }).then(d => setTracker(d)).catch(e => {
        set('log', e.message);
    });
});

document.querySelector('button[name="set"]').addEventListener('click', () => {
    fetch('/tracker', {
        method: 'PUT',
        headers: { 'Content-type': 'application/json' },
        body: JSON.stringify({
            'aos_time': timestamp('aos_time'),
            'aos_az': parseInt(document.querySelector('#aos_az').value) || 0,
            'aos_el': parseInt(document.querySelector('#aos_el').value) || 0,
            'los_time': timestamp('los_time'),
            'los_az': parseInt(document.querySelector('#los_az').value) || 0,
            'los_el': parseInt(document.querySelector('#los_el').value) || 0,
            'mel_el': parseInt(document.querySelector('#mel_el').value) || 0,
        }),
    }).then(r => {
        if (!r.ok) throw new Error('tracker PUT failed');
        return r.json();
    }).then(d => setTracker(d)).catch(e => {
        set('log', e.message);
    });
});

function setTime() {
    fetch('/time', {
        method: 'POST',
        headers: { 'Content-type': 'application/json' },
        body: JSON.stringify({
            'now': (new Date()).getTime(),
        }),
    }).then(r => {
        if (!r.ok) throw new Error('time SET failed');
        return r.json();
    }).catch(e => {
        set('log', e.message);
    });
}

setInterval(() => {
    setTime();
}, 10000);

setTime();