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

function setControl(data) {
    const date = new Date(data.timestamp * 1000);
    set('az_actuator', data.az_degree.toFixed(2));
    set('el_actuator', data.el_degree.toFixed(2));
    set('az_sensor', data.az_sensor.toFixed(2));
    set('el_sensor', data.el_sensor.toFixed(2));
    set('time', date.getHours().toString().padStart(2, '0') +
        date.getMinutes().toString().padStart(2, '0') +
        date.getSeconds().toString().padStart(2, '0'));
}

function setPass(data) {
    const date = new Date(data.timestamp * 1000);
    set('aos_az', data.aos_az);
    set('aos_el', data.aos_el);
    set('los_az', data.los_az);
    set('los_el', data.los_el);
    set('max_el', data.max_el);
    set('state', data.state);
    set('time', String(date.getHours()).padStart(2, '0') + ':' +
        String(date.getMinutes()).padStart(2, '0') + ':' +
        String(date.getSeconds()).padStart(2, '0'));
}

setInterval(function () {
    fetch('/data').then(response => response.json()).then(data => setControl(data));
}, 10000);

let debug = document.querySelector('#debug');

document.querySelectorAll('button[data-axis][data-target]').forEach(element => {
    element.addEventListener('click', () => {
        let url = new URL('/move');
        url.searchParams.append('axis', element.dataset.axis);
        url.searchParams.append('target', element.dataset.target);
        fetch(url).then(response => response.json()).then(data => setControl(data));
    });
});

document.querySelectorAll('button[data-axis][data-step]').forEach(element => {
    element.addEventListener('click', () => {
        let url = new URL('/step');
        url.searchParams.append('axis', element.dataset.axis);
        url.searchParams.append('step', element.dataset.step);
        fetch(url).then(response => response.json()).then(data => setControl(data));
    });
});

document.querySelectorAll('button[data-zero]').forEach(element => {
    element.addEventListener('click', () => {
        let url = new URL('/zero');
        url.searchParams.append('axis', element.dataset.zero);
        fetch(url).then(response => response.json()).then(data => setControl(data));
    });
});

document.querySelector('button[name="execute"]').addEventListener('click', () => {
    fetch('/execute').then(response => response.json()).then(data => setPass(data));
});

document.querySelector('button[name="clear"]').addEventListener('click', () => {
    fetch('/clear').then(response => response.json()).then(data => setPass(data));
});

document.querySelector('button[name="set"]').addEventListener('click', () => {
    const data = {
        'aos': timestamp('aos'),
        'los': timestamp('los'),
        'aos_az': parseInt(document.querySelector('#aos_az').value) || 0,
        'aos_el': parseInt(document.querySelector('#aos_el').value) || 0,
        'los_az': parseInt(document.querySelector('#los_az').value) || 0,
        'los_el': parseInt(document.querySelector('#los_el').value) || 0,
        'max_el': parseInt(document.querySelector('#max_el').value) || 0,
    };

    fetch('/set', {
        method: 'POST',
        headers: { 'Content-type': 'application/json' },
        body: JSON.stringify(data),
    }).then(response => response.json()).then(data => setPass(data));
});