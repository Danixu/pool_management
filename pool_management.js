//const {} = require('zigbee-herdsman-converters/lib/modernExtend');
//const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
//const tz = require('zigbee-herdsman-converters/converters/toZigbee');
//const exposes = require('zigbee-herdsman-converters/lib/exposes');
//const reporting = require('zigbee-herdsman-converters/lib/reporting');
//const extend = require('zigbee-herdsman-converters/lib/extend');
//const ota = require('zigbee-herdsman-converters/lib/ota');
//const tuya = require('zigbee-herdsman-converters/lib/tuya');
//const {} = require('zigbee-herdsman-converters/lib/tuya');
//const utils = require('zigbee-herdsman-converters/lib/utils');
//const globalStore = require('zigbee-herdsman-converters/lib/store');
//const e = exposes.presets;
//const ea = exposes.access;

const {temperature, numeric} = require('zigbee-herdsman-converters/lib/modernExtend');
//const {binary, enumLookup, forcePowerSource, numeric, onOff, customTimeResponse, battery} = require('zigbee-herdsman-converters/lib/modernExtend');

function ph() {
    return numeric({
        name: 'PH',
        cluster: 64777,
        attribute: {ID: 0},
        reporting: {min: '10_SECONDS', max: '1_HOUR', change: 1},
        description: 'PH of the swiming pool',
        //unit: 'ph',
        scale: 100,
        access: 'STATE_GET',
        entityCategory: 'diagnostic'
    });
}


function chlorine() {
    return numeric({
        name: 'Chlorine',
        cluster: 64794,
        attribute: {ID: 0},
        reporting: {min: '10_SECONDS', max: '1_HOUR', change: 1},
        description: 'Chlorine in the swiming pool',
        scale: 100,
        access: 'STATE_GET',
        entityCategory: 'diagnostic'
    });
}


const definition = {
    zigbeeModel: ['Pool.Management'],
    model: 'Pool.Management',
    vendor: 'Danixu',
    description: 'Pool Management device to measure the chlorine and ph values',
    extend: [
        temperature(),
        ph(),
        chlorine(),
    ],
};

module.exports = definition;
