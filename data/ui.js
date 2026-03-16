function timestamp(id) {
    let d = new Date();
    let t = document.querySelector('#' + id).value || String(d.getHours()).padStart(2, 0) + ':' + String(d.getMinutes()).padStart(2, 0);
    let [h, m] = t.split(':').map(Number);
    d.setHours(h, m, 0, 0);
    return d.getTime();
}

function set(id, value) {
    document.querySelector('#' + id).innerText = value;
}

function setActuator(data) {
    const date = new Date(data.ts * 1000);
    set('az_deg', data.az_deg.toFixed(2));
    set('el_deg', data.el_deg.toFixed(2));
    set('az_pos', data.az_pos.toFixed(2));
    set('el_pos', data.el_pos.toFixed(2));
    set('ts', date.getHours().toString().padStart(2, '0') +
        date.getMinutes().toString().padStart(2, '0') +
        date.getSeconds().toString().padStart(2, '0'));
}

function setTracker(data) {
    const date = new Date(data.ts * 1000);
    // TODO: set aos & los
    set('aos_az', data.aos_az);
    set('aos_el', data.aos_el);
    set('los_az', data.los_az);
    set('los_el', data.los_el);
    set('max_el', data.max_el);
    set('state', data.state);
    set('cd', data.cd);
    set('ts', String(date.getHours()).padStart(2, '0') + ':' +
        String(date.getMinutes()).padStart(2, '0') + ':' +
        String(date.getSeconds()).padStart(2, '0'));
}

document.querySelectorAll('button[name="move"]').forEach(el => {
    el.addEventListener('click', () => {
        fetch('/actuator', {
            method: 'POST',
            headers: { 'Content-type': 'application/json' },
            body: JSON.stringify({
                mode: el.dataset.mode,
                az: parseFloat('az' === el.dataset.axis ? el.value : 0),
                el: parseFloat('el' === el.dataset.axis ? el.value : 0),
            }),
        }).then(r => {
            if (!r.ok) throw new Error(r.status);
            return r.json();
        }).then(d => setActuator(d)).catch(e => {
            console.log(e.message);
        });
    });
});

document.querySelectorAll('button[name="zero"]').forEach(el => {
    el.addEventListener('click', () => {
        fetch('/actuator', {
            method: 'DELETE',
            headers: { 'Content-type': 'application/json' },
        }).then(r => {
            if (!r.ok) throw new Error(r.status);
            return r.json();
        }).then(d => setActuator(d)).catch(e => {
            console.log(e.message);
        });
    });
});

document.querySelector('button[name="execute"]').addEventListener('click', () => {
    fetch('/tracker', {
        method: 'POST',
        headers: { 'Content-type': 'application/json' },
    }).then(r => {
        if (!r.ok) throw new Error(r.status);
        return r.json();
    }).then(d => setTracker(d)).catch(e => {
        console.log(e.message);
    });
});

document.querySelector('button[name="clear"]').addEventListener('click', () => {
    fetch('/tracker', {
        method: 'DELETE',
        headers: { 'Content-type': 'application/json' },
    }).then(r => {
        if (!r.ok) throw new Error(r.status);
        return r.json();
    }).then(d => setTracker(d)).catch(e => {
        console.log(e.message);
    });
});

document.querySelector('button[name="set"]').addEventListener('click', () => {
    fetch('/tracker', {
        method: 'PATCH',
        headers: { 'Content-type': 'application/json' },
        body: JSON.stringify({
            'aos': timestamp('aos'),
            'los': timestamp('los'),
            'aos_az': parseInt(document.querySelector('#aos_az').value) || 0,
            'aos_el': parseInt(document.querySelector('#aos_el').value) || 0,
            'los_az': parseInt(document.querySelector('#los_az').value) || 0,
            'los_el': parseInt(document.querySelector('#los_el').value) || 0,
            'max_el': parseInt(document.querySelector('#max_el').value) || 0,
        }),
    }).then(r => {
        if (!r.ok) throw new Error(r.status);
        return r.json();
    }).then(d => setTracker(d)).catch(e => {
        console.log(e.message);
    });
});