<template>
    <Layout>
        <template #not-found>
            <NotFound/>
        </template>
    </Layout>
</template>

<script setup>
import DefaultTheme from 'vitepress/theme';
import NotFound from './NotFound.vue';

const { Layout } = DefaultTheme
</script>