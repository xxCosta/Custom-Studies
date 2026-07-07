import { $ } from 'bun'
import PouchDB from 'pouchdb'

const db = new PouchDB('test1');
let doc = {
  "_id": "ben234",
  "name": "ben",
  "age": 28
}


//db.put(doc)

db.get("ben234").then((doc) => {
  console.log(doc)
})

await $`cowsay LOGSERVER`
Bun.serve({
  hostname: '0.0.0.0',
  port: 5555,
  routes: {
    "/": () => {
      console.log("ping")
      return new Response("PONG")
    },
    "/addTrade": async (req) => {
      const url = new URL(req.url)
      let params = url.searchParams
      params.forEach((k, v) => {
        console.log(k, ":", v)
      })



      return new Response("hello")
    },
  },
  error(error) {
    return new Response(error.stack);
  }
})

