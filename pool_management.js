//const {} = require('zigbee-herdsman-converters/lib/modernExtend');
const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
//const extend = require('zigbee-herdsman-converters/lib/extend');
//const ota = require('zigbee-herdsman-converters/lib/ota');
//const tuya = require('zigbee-herdsman-converters/lib/tuya');
//const {} = require('zigbee-herdsman-converters/lib/tuya');
//const utils = require('zigbee-herdsman-converters/lib/utils');
//const globalStore = require('zigbee-herdsman-converters/lib/store');
const e = exposes.presets;
const ea = exposes.access;

const { toZigbee, fromZigbee, temperature, onOff } = require('zigbee-herdsman-converters/lib/modernExtend');

//const {binary, enumLookup, forcePowerSource, numeric, onOff, customTimeResponse, battery} = require('zigbee-herdsman-converters/lib/modernExtend');

const fzLocal = {
    ph: {
        cluster: '64800', //This part is important
        type: ['readResponse', 'attributeReport'],
        convert: (model, msg, publish, options, meta) => {
            if (msg.data.hasOwnProperty('0')) {
                const ph = parseFloat(msg.data['0'] / 100.0);
                return { ph };
            }
        },
    },

    chlorine: {
        cluster: '64784', //This part is important
        type: ['readResponse', 'attributeReport'],
        convert: (model, msg, publish, options, meta) => {
            if (msg.data.hasOwnProperty('0')) {
                const chlorine = parseFloat(msg.data['0'] / 100.0);
                return { chlorine };
            }
        },
    },
};


const definition = {
    zigbeeModel: ['Pool Management'],
    model: 'Pool Management',
    vendor: 'Danixu',
    description: 'Pool Management device to measure the chlorine and ph values',
    endpoint: (device) => {
        return {
            general: 1,
            pump: 2,
            ph: 3,
            chlorine: 4,
            algaecide: 5
        };
    },
    /*
    toZigbee: [
        tz.on_off
    ],
    */
    fromZigbee: [
        fz.on_off,
        fzLocal.ph,
        fzLocal.chlorine
    ],
    exposes: [
        e.numeric('ph', ea.STATE).withLabel('PH')
            .withDescription('PH value in the swimming pool'),
        e.numeric('chlorine', ea.STATE)
            .withDescription("Chlorine level in the swimming pool")
    ],
    extend: [
        temperature(),
        onOff({ powerOnBehavior: false, endpointNames: ["general", "pump", "ph", "chlorine", "algaecide"] }).pepe(),
        //onOff({ powerOnBehavior: false, endpointNames: ["pump"]}),
        //chlorine(),
        //ph()
    ],
    //meta: { multiEndpoint: true },
    configure: async (device, coordinatorEndpoint) => {
        const endpoint_1 = device.getEndpoint(1);
        const binds_1 = ['genBasic', 'msTemperatureMeasurement', 'genOnOff', 'genTime'];
        await reporting.bind(endpoint_1, coordinatorEndpoint, binds_1);
        const endpoint_2 = device.getEndpoint(2);
        const binds_2 = ['genOnOff'];
        await reporting.bind(endpoint_2, coordinatorEndpoint, binds_2);
        const endpoint_3 = device.getEndpoint(3);
        const binds_3 = ['genOnOff', 'genLevelCtrl', 64800];
        await reporting.bind(endpoint_3, coordinatorEndpoint, binds_3);
        const endpoint_4 = device.getEndpoint(4);
        const binds_4 = ['genOnOff', 'genLevelCtrl', 64784];
        await reporting.bind(endpoint_4, coordinatorEndpoint, binds_4);
        const endpoint_5 = device.getEndpoint(5);
        const binds_5 = ['genOnOff', 'genLevelCtrl'];
        await reporting.bind(endpoint_5, coordinatorEndpoint, binds_5);
    },
};

module.exports = definition;
