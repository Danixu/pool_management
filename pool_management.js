//const {} = require('zigbee-herdsman-converters/lib/modernExtend');
const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
//const reporting = require('zigbee-herdsman-converters/lib/reporting');
//const extend = require('zigbee-herdsman-converters/lib/extend');
//const ota = require('zigbee-herdsman-converters/lib/ota');
//const tuya = require('zigbee-herdsman-converters/lib/tuya');
//const {} = require('zigbee-herdsman-converters/lib/tuya');
//const utils = require('zigbee-herdsman-converters/lib/utils');
//const globalStore = require('zigbee-herdsman-converters/lib/store');
const e = exposes.presets;
const ea = exposes.access;

const {
    Fz,
    Tz,
    ModernExtend,
    Expose,
} = 'zigbee-herdsman-converters/lib/types';

const { onOff, deviceAddCustomCluster, temperature, numeric } = require('zigbee-herdsman-converters/lib/modernExtend');
const Zcl = require('zigbee-herdsman').Zcl;

//const {binary, enumLookup, forcePowerSource, numeric, onOff, customTimeResponse, battery} = require('zigbee-herdsman-converters/lib/modernExtend');

function ph() {
    return numeric({
        name: 'PH Value',
        cluster: "phValue",
        attribute: "measuredValue",
        valueMin: 700,
        valueMax: 740,
        reporting: { min: '10_SECONDS', max: '1_HOUR', change: 1 },
        description: 'PH of the swiming pool',
        //unit: 'ph',
        scale: 100,
        access: 'STATE_GET',
        entityCategory: 'diagnostic'
    });
}

function phTargetValue() {
    return numeric({
        name: 'PH Target Value',
        cluster: "phTargetValue",
        attribute: "targetValue",
        valueMin: 7.00,
        valueMax: 7.40,
        reporting: { min: '10_SECONDS', max: '1_HOUR', change: 1 },
        description: 'Target value for the PH level',
        //unit: 'ph',
        valueStep: 0.01,
        scale: 100,
        access: 'STATE_SET',
        entityCategory: 'config'
    });
}

function phDeposit() {
    return numeric({
        name: 'PH Deposit level',
        cluster: "phDepositValue",
        attribute: "measuredValue",
        valueMin: 0,
        valueMax: 100,
        reporting: { min: '10_SECONDS', max: '1_HOUR', change: 1 },
        description: 'PH liquid deposit level',
        unit: '%',
        //scale: 100,
        access: 'STATE_GET',
        entityCategory: 'diagnostic'
    });
}


function chlorine() {
    return numeric({
        name: 'Chlorine Value',
        cluster: "chlorineValue",
        attribute: "measuredValue",
        valueMin: 0,
        valueMax: 1400,
        reporting: { min: '10_SECONDS', max: '1_HOUR', change: 1 },
        description: 'Chlorine in the swiming pool',
        scale: 100,
        access: 'STATE_GET',
        entityCategory: 'diagnostic'
    });
}

function chlorineTargetValue() {
    return numeric({
        name: 'Chlorine Target Value',
        cluster: "chlorineTargetValue",
        attribute: "targetValue",
        valueMin: 7.00,
        valueMax: 7.40,
        reporting: { min: '10_SECONDS', max: '1_HOUR', change: 1 },
        description: 'Target value of the chlorine level',
        //unit: 'ph',
        valueStep: 0.01,
        scale: 100,
        access: 'STATE_SET',
        entityCategory: 'config'
    });
}

function chlorineDeposit() {
    return numeric({
        name: 'Chlorine Deposit Level',
        cluster: "chlorineDepositValue",
        attribute: "measuredValue",
        valueMin: 0,
        valueMax: 100,
        reporting: { min: '10_SECONDS', max: '1_HOUR', change: 1 },
        description: 'Chlorine liquid deposit level',
        unit: '%',
        //scale: 100,
        access: 'STATE_GET',
        entityCategory: 'diagnostic'
    });
}

function algaecideDeposit() {
    return numeric({
        name: 'Algaecide Deposit Level',
        cluster: "algaecideDepositValue",
        attribute: "measuredValue",
        valueMin: 0,
        valueMax: 100,
        reporting: { min: '10_SECONDS', max: '1_HOUR', change: 1 },
        description: 'Algaecide liquid deposit level',
        unit: '%',
        //scale: 100,
        access: 'STATE_GET',
        entityCategory: 'diagnostic'
    });
}


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
    //fromZigbee: [],
    //toZigbee: [],
    //exposes: [],
    extend: [
        deviceAddCustomCluster('algaecideDepositValue', {
            ID: 0xfd32,
            //manufacturerCode: 0x4c44,
            attributes: {
                measuredValue: { ID: 0x0000, type: Zcl.DataType.UINT8 },
            },
            commands: {},
            commandsResponse: {},
        }),
        deviceAddCustomCluster('chlorineValue', {
            ID: 0xfd10,
            //manufacturerCode: 0x4c44,
            attributes: {
                measuredValue: { ID: 0x0000, type: Zcl.DataType.UINT16 },
                maxeasuredValue: { ID: 0x0001, type: Zcl.DataType.UINT16 },
                minMeasuredValue: { ID: 0x0002, type: Zcl.DataType.UINT16 }
            },
            commands: {},
            commandsResponse: {},
        }),
        deviceAddCustomCluster('chlorineTargetValue', {
            ID: 0xfd11,
            //manufacturerCode: 0x4c44,
            attributes: {
                targetValue: { ID: 0x0000, type: Zcl.DataType.UINT16 },
            },
            commands: {},
            commandsResponse: {},
        }),
        deviceAddCustomCluster('chlorineDepositValue', {
            ID: 0xfd12,
            //manufacturerCode: 0x4c44,
            attributes: {
                measuredValue: { ID: 0x0000, type: Zcl.DataType.UINT8 },
            },
            commands: {},
            commandsResponse: {},
        }),
        deviceAddCustomCluster('phValue', {
            ID: 0xfd20,
            //manufacturerCode: 0x4c44,
            attributes: {
                measuredValue: { ID: 0x0000, type: Zcl.DataType.UINT16 },
                maxeasuredValue: { ID: 0x0001, type: Zcl.DataType.UINT16 },
                minMeasuredValue: { ID: 0x0002, type: Zcl.DataType.UINT16 }
            },
            commands: {},
            commandsResponse: {},
        }),
        deviceAddCustomCluster('phTargetValue', {
            ID: 0xfd21,
            //manufacturerCode: 0x4c44,
            attributes: {
                targetValue: { ID: 0x0000, type: Zcl.DataType.UINT16 },
            },
            commands: {},
            commandsResponse: {},
        }),
        deviceAddCustomCluster('phDepositValue', {
            ID: 0xfd22,
            //manufacturerCode: 0x4c44,
            attributes: {
                measuredValue: { ID: 0x0000, type: Zcl.DataType.UINT8 },
            },
            commands: {},
            commandsResponse: {},
        }),
        temperature(),
        // For now the onOff switches doesn't allows to split into more than one group
        onOff({ powerOnBehavior: false, endpointNames: ["general", "pump", "ph", "chlorine", "algaecide"] }),
        algaecideDeposit(),
        chlorine(),
        chlorineTargetValue(),
        chlorineDeposit(),
        ph(),
        phTargetValue(),
        phDeposit(),
    ],
    meta: { multiEndpoint: true },
    /*
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
    */
};

module.exports = definition;

