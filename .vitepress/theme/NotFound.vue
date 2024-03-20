<template>
    <div class="container">
        <div class="head">跳转 GitHub 中...</div>
        <p>{{ decodeURI(to) }}</p>
    </div>
</template>

<script setup>
import { useRouter, useData } from 'vitepress';
import { repo_name, repo_url } from './params';

const data = useData();
data.page.value.title = '跳转 GitHub 中...';

const router = useRouter();
const to = repo_url + 'blob/main' + router.route.path.substring(repo_name.length);
console.log(to);
location.replace(to);
</script>

<style scoped>
.container {
    position: absolute;
    left: 50%;
    top: 50%;
    transform: translate(-50%, -100%);
    border: solid gray 1px;
    padding: 20px;
    border-radius: 5px;
    box-shadow: 0px 0px 1px;
}

.head {
    color: rgb(111, 111, 238);
    font-size: larger;
    font-weight: bolder;
    margin-bottom: 1em;
}
</style>