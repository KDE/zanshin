Feature: Tag creation
  As someone using tasks and notes
  I can create a tag
  In order to give them some semantic

@wip
  Scenario: New contexts appear in the list
    Given I display the available pages
    When I add a "tag" named "Optional"
    And I list the items
    Then the list is:
       | display                           | icon                |
       | Contexts                          | folder              |
       | Contexts / Errands                | view-pim-tasks      |
       | Contexts / Internet               | view-pim-tasks      |
       | Contexts / Online                 | view-pim-tasks      |
       | Inbox                             | mail-folder-inbox   |
       | Projects                          | folder              |
       | Projects / Backlog                | view-pim-tasks      |
       | Projects / Prepare talk about TDD | view-pim-tasks      |
       | Projects / Read List              | view-pim-tasks      |
       | Tags / Optional                   | view-pim-tasks      |

