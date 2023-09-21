#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/errno.h>

struct job_instance {
    int delay;
    int cnt;
    struct delayed_work job;
};

struct gpio_sample_data {
    struct gpio_desc* desc;
    int gpio_state;
};

static struct gpio_sample_data g_gpio_sample_data;
static struct job_instance g_jobs[5];
static int delay_mode = 0;

static void gpio_sample_blink(struct work_struct*wk) {
    struct job_instance *w = container_of((void*)wk, struct job_instance, job);
    int state = gpiod_get_value(g_gpio_sample_data.desc);
    msleep(w->delay);
    g_gpio_sample_data.gpio_state = !state;
    pr_info("Inverting gpio output to %d\n", g_gpio_sample_data.gpio_state);
    gpiod_set_value(g_gpio_sample_data.desc, g_gpio_sample_data.gpio_state);

    switch (delay_mode) {
        case 0:
        while (--w->cnt) {
            state = !state;
            gpiod_set_value(g_gpio_sample_data.desc, state);
        }
        break;

        case 1:
        while (--w->cnt) {
            ndelay(1);
            state = !state;
            gpiod_set_value(g_gpio_sample_data.desc, state);
        }
        break;

        case 2:
        schedule_delayed_work(&g_jobs[0].job, g_jobs[0].delay);
        break;

        default:
            pr_err("Invalid delay mode %d\n", delay_mode);
    }
    g_gpio_sample_data.gpio_state = state;
}

static int gpio_sample_probe(struct platform_device *pdev)
{
    pr_info("GPIO Sample consumer being probed.\n");
    g_gpio_sample_data.desc = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(g_gpio_sample_data.desc)) {
        pr_err("Failed to get GPIO\n");
        return -EINVAL;
    }
    g_jobs[0].delay = 1;
    g_jobs[0].cnt = 5000 / g_jobs[0].delay;
    INIT_DELAYED_WORK(&(g_jobs[0].job), gpio_sample_blink);
    schedule_delayed_work(&g_jobs[0].job, g_jobs[0].delay);
    return 0;
}

static int gpio_sample_remove(struct platform_device *pdev) {
    pr_info("GPIO Sample consumer being removed.\n");
    gpiod_set_value(g_gpio_sample_data.desc, 0);
    devm_gpiod_put(&(pdev->dev), g_gpio_sample_data.desc);
    return 0;
}

static struct of_device_id gpio_sample_of_match[] = {
    { .compatible = "gpio-sample", },
    { /* end of list */ },
};

static struct platform_driver gpio_sample_driver = {
    .probe = gpio_sample_probe,
    .remove = gpio_sample_remove,
    .driver = {
        .name = "gpio-sample",
        .owner = THIS_MODULE,
        .of_match_table = gpio_sample_of_match,
    },
};

module_platform_driver(gpio_sample_driver);

module_param(delay_mode, int, 0);
MODULE_LICENSE("GPL");
