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

var assert = require('assert');

onmessage = function (msg) {
  switch (msg.data.aTopic) {
    case 'do_sendWorkerArrBuff':
      sendWorkerArrBuff(msg.data.aBuf)
      break;
    default:
      throw 'no aTopic on incoming message to ThreadWorker';
  }
}

function sendWorkerArrBuff(aBuf) {
    console.info('from worker, PRE send back aBuf.byteLength:', aBuf.byteLength);
    assert.equal(aBuf.byteLength, 8);

    postMessage({aTopic:'do_sendMainArrBuff', aBuf:aBuf}, [aBuf]);

    console.info('from worker, POST send back aBuf.byteLength:', aBuf.byteLength);
    assert.equal(aBuf.byteLength, 0);
}
