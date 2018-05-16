/*
 * Copyright (C) 2017 Alibaba Group Holding Limited. All Rights Reserved.
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the 'Software'), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

'use strict';

var common = require('../common');
var assert = require('assert');

var worker = new ThreadWorker("test/thread-worker/ab-postmessage.js")

worker.onmessage = function(env) {
  var data = env.data;
  console.log('Get msg from worker: ' + data.aTopic)
  assert.equal(data.aTopic, "do_sendMainArrBuff");
  worker.terminate();
}

var arrBuf = new ArrayBuffer(8);
console.info('arrBuf.byteLength pre transfer:', arrBuf.byteLength);
assert.equal(arrBuf.byteLength, 8);

worker.postMessage(
    {
        aTopic: 'do_sendWorkerArrBuff',
        aBuf: arrBuf
    },
    [
        arrBuf
    ]
);

console.info('arrBuf.byteLength post transfer:', arrBuf.byteLength);
assert.equal(arrBuf.byteLength, 0);
